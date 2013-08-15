/**
 * @file fastcgi.c
 * @purpose Runs the FCGI request loop to handle web interface requests.
 * 
 * <stdio.h> should not be included, because these functions are handled by
 * fcgi_stdio.h. If included, it must be included after fcgi_stdio.h.
 */
 
#include <fcgi_stdio.h>
#include <string.h>
#include <stdlib.h>

/*
	But the suggestion was: FunctionName, variable_name (local or member),
    Structure, ENUMVALUE, Extern_FunctionName, g_global
*/

//Replace with whatever holds the 'data'
typedef struct Data Data;
enum {RESPONSE_OK = 200, RESPONSE_BADREQUEST = 400,
	  RESPONSE_UNAUTHORIZED = 401};

typedef void (*ModuleHandler) (Data *data, char *params);

/**
 * Extracts a key/value pair from a request string.
 * Note that the input is modified by this function.
 * @param in The string from which to extract the pair
 * @param key A pointer to a variable to hold the key string
 * @param value A pointer to a variable to hold the value string
 * @return A pointer to the start of the next search location, or NULL if
 *         the EOL is reached.
 */
static char *KeyPair(char *in, const char **key, const char **value) {
	char *next, *split;
	if (!in || !*in) { //Invalid input or string is EOL
		return NULL;
	}

	*key = in;
	//Must be first so value will be empty if it's not specified
	if ((next = strchr(in, '&'))) {
		*next++ = 0;
	} else { //Don't return NULL as current pair needs to be returned
		next = "";
	}
	if ((split = strchr(in, '='))) {
		*split++ = 0;
		*value = split; 
		return next;
	}
	//Split was not found, set to default value
	*value = "";
	return next ? next : "";
}

static void BeginResponse(int response_code, const char *module) {
	switch (response_code) {
		case RESPONSE_OK:
			break;
		case RESPONSE_UNAUTHORIZED:
			printf("Status: 401 Unauthorized\r\n");
			break;
		default:
			printf("Status: 400 Bad Request\r\n");
	}
	printf("Content-type: application/json; charset=utf-8\r\n\r\n");
	printf("{\r\n");
	printf("\t\"module\" : \"%s\"", module);
}

static void BuildResponse(const char *key, const char *value) {
	printf(",\r\n\t\"%s\" : \"%s\"", key, value);
}

static void EndResponse() {
	printf("\r\n}\r\n");
}

static void SensorsHandler(Data *data, char *params) {
	const char *key, *value;
	BeginResponse(RESPONSE_OK, "sensors");
 
   	while ((params = KeyPair(params, &key, &value))) {
   		BuildResponse(key, value);
   	}
   	EndResponse();
	
}

void FCGI_RequestLoop (Data *data)
{
	int count = 0;
	while (FCGI_Accept() >= 0)   {
		ModuleHandler module_handler = NULL;
		char module[BUFSIZ], params[BUFSIZ];
		
		//strncpy doesn't zero-truncate properly
		snprintf(module, BUFSIZ, "%s", getenv("DOCUMENT_URI_LOCAL"));
		snprintf(params, BUFSIZ, "%s", getenv("QUERY_STRING"));
		
		//Remove trailing slashes (if present) from module query
		size_t lastchar = strlen(module) - 1;
		if (lastchar > 0 && module[lastchar] == '/')
			module[lastchar] = '\0';
		

		if (!strcmp("sensors", module)) {
			module_handler = SensorsHandler;
		} else if (!strcmp("admin", module)) {
			//module_handler = AdminHandlerReplace with pointer to admin handler
		}

		if (module_handler) {
			module_handler(data, params);
		} else {
			char buf[BUFSIZ];
			
			BeginResponse(400, module);
			BuildResponse("description", "400 Invalid response");
			snprintf(buf, BUFSIZ, "%d", count);
			BuildResponse("request-number", buf);
			BuildResponse("params", params);
			BuildResponse("host", getenv("SERVER_HOSTNAME"));
			EndResponse();
		}

		count++;
		//Debgging:
		//printf("Module: %s, Params: %s<br>\n", module, params);
		//printf("Request number %d, host <i>%s</i>\n",
		//	count++, getenv("SERVER_HOSTNAME"));
	}
}

int main(int argc, char *argv[]) {
	FCGI_RequestLoop(NULL);
}
