/**
 * @file fastcgi.c
 * @purpose Runs the FCGI request loop to handle web interface requests.
 * 
 * fcgi_stdio.h must be included before all else so the stdio function
 * redirection works ok.
 */
 
#include <fcgi_stdio.h>
#include "fastcgi.h"
//#include "common.h"

/**
 * Extracts a key/value pair from a request string.
 * Note that the input is modified by this function.
 * @param in The string from which to extract the pair
 * @param key A pointer to a variable to hold the key string
 * @param value A pointer to a variable to hold the value string
 * @return A pointer to the start of the next search location, or NULL if
 *         the EOL is reached.
 */
char *FCGI_KeyPair(char *in, const char **key, const char **value)
{
	char *ptr;
	if (!in || !*in) { //Invalid input or string is EOL
		return NULL;
	}

	*key = in;
	//Find either = or &, whichever comes first
	if ((ptr = strpbrk(in, "=&"))) {
		if (*ptr == '&') { //No value specified
			*value = ptr;
			*ptr++ = 0;
		} else {
			//Stopped at an '=' sign
			*ptr++ = 0;
			*value = ptr;
			if ((ptr = strchr(ptr,'&'))) {
				*ptr++ = 0;
			} else {
				ptr = "";
			}
		}
	} else { //No value specified and no other pair
		ptr = "";
		*value = ptr;
	}
	return ptr;
}

/**
 * Begins a response to the client in JSON format.
 * @param status_code The HTTP status code to be returned.
 * @param module The name of the module that initiated the response.
 */
void FCGI_BeginJSON(StatusCodes status_code, const char *module)
{
	switch (status_code) {
		case STATUS_OK:
			break;
		case STATUS_UNAUTHORIZED:
			printf("Status: 401 Unauthorized\r\n");
			break;
		default:
			printf("Status: 400 Bad Request\r\n");
	}
	printf("Content-type: application/json; charset=utf-8\r\n\r\n");
	printf("{\r\n");
	printf("\t\"module\" : \"%s\"", module);
}

/**
 * Adds a key/value pair to a JSON response. The response must have already
 * been initiated by FCGI_BeginJSON. Note that characters are not escaped.
 * @param key The key of the JSON entry
 * &param value The value associated with the key.
 */
void FCGI_BuildJSON(const char *key, const char *value)
{
	printf(",\r\n\t\"%s\" : \"%s\"", key, value);
}

/**
 * Ends a JSON response that was initiated by FCGI_BeginJSON.
 */
void FCGI_EndJSON() 
{
	printf("\r\n}\r\n");
}

/**
 * Main FCGI request loop that receives/responds to client requests.
 * @param data A data field to be passed to the selected module handler.
 */ 
void FCGI_RequestLoop (void *data)
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
			module[lastchar] = 0;
		

		if (!strcmp("sensors", module)) {
			//module_handler = Handler_Sensors;
		} else if (!strcmp("actuators", module)) {
			
		}

		if (module_handler) {
			module_handler(data, params);
		} else {
			char buf[BUFSIZ];
			
			FCGI_BeginJSON(400, module);
			FCGI_BuildJSON("description", "400 Invalid response");
			snprintf(buf, BUFSIZ, "%d", count);
			FCGI_BuildJSON("request-number", buf);
			FCGI_BuildJSON("params", params);
			FCGI_BuildJSON("host", getenv("SERVER_HOSTNAME"));
			FCGI_EndJSON();
		}

		count++;
	}
}
