/**
 * @file fastcgi.c
 * @purpose Runs the FCGI request loop to handle web interface requests.
 *
 * fcgi_stdio.h must be included before all else so the stdio function
 * redirection works ok.
 */

#include <fcgi_stdio.h>
#include <openssl/sha.h>
#include <time.h>

#include "common.h"
#include "sensor.h"
#include "log.h"
#include "options.h"

#define LOGIN_TIMEOUT 180

struct FCGIContext {
	/**The time of last valid logged-in user access*/
	time_t login_timestamp;
	char login_key[41];
	char login_ip[16];
	/**The name of the current module**/
	const char *current_module;
	/**For debugging purposes?**/
	int response_number;
};

/**
 * Handles user logins.
 * @param context The context to work in
 * @param params User specified parameters
 */
static void LoginHandler(FCGIContext *context, char *params) {
	const char *key, *value;
	bool force = 0, end = 0;

	while ((params = FCGI_KeyPair(params, &key, &value))) {
		if (!strcmp(key, "force"))
			force = !force;
		else if (!strcmp(key, "end"))
			end = !end;
	}

	if (end) {
		*(context->login_key) = 0;
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_EndJSON();
		return;
	}

	time_t now = time(NULL);
	if (force || !*(context->login_key) || 
	   (now - context->login_timestamp > LOGIN_TIMEOUT)) 
	{
		SHA_CTX sha1ctx;
		unsigned char sha1[20];
		int i = rand();

		SHA1_Init(&sha1ctx);
		SHA1_Update(&sha1ctx, &now, sizeof(now));
		SHA1_Update(&sha1ctx, &i, sizeof(i));
		SHA1_Final(sha1, &sha1ctx);

		context->login_timestamp = now;
		for (i = 0; i < 20; i++)
			sprintf(context->login_key + i * 2, "%02x", sha1[i]);
		snprintf(context->login_ip, 16, "%s", getenv("REMOTE_ADDR"));
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("key", context->login_key);
		FCGI_EndJSON();
	} else {
		char buf[128];
		strftime(buf, 128, "%H:%M:%S %d-%m-%Y",
			localtime(&(context->login_timestamp))); 
		FCGI_BeginJSON(context, STATUS_UNAUTHORIZED);
		FCGI_JSONPair("description", "Already logged in");
		FCGI_JSONPair("user", context->login_ip); 
		FCGI_JSONPair("time", buf);
		FCGI_EndJSON();
	}
}

/*TODO: Remove and replace with the actual actuator code*/
static void ActuatorHandler(FCGIContext *context, char *params) {
	const char *key, *value, *loginkey = NULL;
	while ((params = FCGI_KeyPair(params, &key, &value))) {
		if (!strcmp(key, "key")) {
			loginkey = value;
		}
	}
	if (!loginkey || !FCGI_Authorized(context, loginkey)) {
		FCGI_BeginJSON(context, STATUS_UNAUTHORIZED);
		FCGI_JSONPair("description", "Invalid key specified.");
		FCGI_EndJSON();
	} else {
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("description", "Logged in!");
		FCGI_EndJSON();
	}
}

/**
 * Given an FCGIContext, determines if the current user (as specified by
 * the key) is authorized or not. If validated, the context login_timestamp is
 * updated.
 * @param context The context to work in
 * @param key The login key to be validated.
 * @return TRUE if authorized, FALSE if not.
 */
bool FCGI_Authorized(FCGIContext *context, const char *key) {
	time_t now = time(NULL);
	int result = (now - context->login_timestamp) <= LOGIN_TIMEOUT &&
				 !strcmp(context->login_key, key);
	if (result) {
		context->login_timestamp = now; //Update the login_timestamp
	}
	return result;
}

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
 * @param context The context to work in.
 * @param status_code The status code to be returned.
 */
void FCGI_BeginJSON(FCGIContext *context, StatusCodes status_code)
{
	printf("Content-type: application/json; charset=utf-8\r\n\r\n");
	printf("{\r\n");
	printf("\t\"module\" : \"%s\"", context->current_module);
	FCGI_JSONLong("status", status_code);

	// Jeremy: Should we include a timestamp in the JSON; something like this?
	double start_time = g_options.start_time.tv_sec + 1e-6*(g_options.start_time.tv_usec);
	struct timeval now;
	gettimeofday(&now, NULL);
	double current_time = now.tv_sec + 1e-6*(now.tv_usec);
	FCGI_JSONDouble("start_time", start_time);
	FCGI_JSONDouble("current_time", current_time);
	FCGI_JSONDouble("running_time", current_time - start_time);
	
}

/**
 * Adds a key/value pair to a JSON response. The response must have already
 * been initiated by FCGI_BeginJSON. Note that characters are not escaped.
 * @param key The key of the JSON entry
 * &param value The value associated with the key.
 */
void FCGI_JSONPair(const char *key, const char *value)
{
	printf(",\r\n\t\"%s\" : \"%s\"", key, value);
}

/**
 * Similar to FCGI_JSONPair except for signed integer values.
 * @param key The key of the JSON entry
 * @param value The value associated with the key
 */
void FCGI_JSONLong(const char *key, long value)
{
	printf(",\r\n\t\"%s\" : %ld", key, value);
}

/**
 * Similar to FCGI_JsonPair except for floating point values.
 * @param key The key of the JSON entry
 * @param value The value associated with the key
 */
void FCGI_JSONDouble(const char *key, double value)
{
	printf(",\r\n\t\"%s\" : %f", key, value);
}

/**
 * Begins a JSON entry by writing the key. To be used in conjunction
 * with FCGI_JsonValue.
 * @param key The key of the JSON entry
 */
void FCGI_JSONKey(const char *key)
{
	printf(",\r\n\t\"%s\" : ", key);
}

/**
 * Should be used to write out the value of a JSON key. This has
 * the same format as the printf functions. Care should be taken to format
 * the output in valid JSON. 
 */
void FCGI_JSONValue(const char *format, ...)
{
	va_list list;
	va_start(list, format);
	vprintf(format, list);
	va_end(list);
}

/**
 * Ends a JSON response that was initiated by FCGI_BeginJSON.
 */
void FCGI_EndJSON() 
{
	printf("\r\n}\r\n");
}

/**
 * To be used when the input parameters are invalid.
 * Sends a response with HTTP status 400 Bad request, along with
 * JSON data for debugging.
 * @param context The context to work in
 * @param params The parameters that the module handler received.
 */
void FCGI_RejectJSON(FCGIContext *context)
{
	printf("Status: 400 Bad Request\r\n");
	
	FCGI_BeginJSON(context, STATUS_ERROR);
	FCGI_JSONPair("description", "Invalid request");
	FCGI_JSONLong("responsenumber", context->response_number);
	FCGI_JSONPair("params", getenv("QUERY_STRING"));
	FCGI_JSONPair("host", getenv("SERVER_HOSTNAME"));
	FCGI_JSONPair("user", getenv("REMOTE_USER"));
	FCGI_JSONPair("ip", getenv("REMOTE_ADDR"));
	FCGI_EndJSON();
}

/**
 * Main FCGI request loop that receives/responds to client requests.
 * @param data Reserved.
 * @returns NULL (void* required for consistency with pthreads, although at the moment this runs in the main thread anyway)
 * TODO: Get this to exit with the rest of the program!
 */ 
void * FCGI_RequestLoop (void *data)
{
	FCGIContext context = {0};
	
	Log(LOGDEBUG, "First request...");
	//TODO: The FCGI_Accept here is blocking. 
	//		That means that if another thread terminates the program, this thread
	//		 will not terminate until the next request is made.
	while (FCGI_Accept() >= 0) {

		if (Thread_Runstate() != RUNNING)
		{
			//TODO: Yeah... deal with this better :P
			Log(LOGERR, "FIXME; FCGI gets request after other threads have finished.");
			printf("Content-type: text/plain\r\n\r\n+++OUT OF CHEESE ERROR+++\n");
			break;
		}
		
		Log(LOGDEBUG, "Got request #%d", context.response_number);
		ModuleHandler module_handler = NULL;
		char module[BUFSIZ], params[BUFSIZ];
		
		//strncpy doesn't zero-truncate properly
		snprintf(module, BUFSIZ, "%s", getenv("DOCUMENT_URI_LOCAL"));
		snprintf(params, BUFSIZ, "%s", getenv("QUERY_STRING"));
		
		//Remove trailing slashes (if present) from module query
		size_t lastchar = strlen(module) - 1;
		if (lastchar > 0 && module[lastchar] == '/')
			module[lastchar] = 0;
		

		if (!strcmp("login", module)) {
			module_handler = LoginHandler;
		} else if (!strcmp("sensors", module)) {
			module_handler = Sensor_Handler;
		} else if (!strcmp("actuators", module)) {
			module_handler = ActuatorHandler;
		}

		context.current_module = module;
		if (module_handler) {
			module_handler(&context, params);
		} else {
			strncat(module, " [unknown]", BUFSIZ);
			FCGI_RejectJSON(&context);
		}
		context.response_number++;

		Log(LOGDEBUG, "Waiting for request #%d", context.response_number);
	}

	Log(LOGDEBUG, "Thread exiting.");
	Thread_QuitProgram(false);
	// NOTE: Don't call pthread_exit, because this runs in the main thread. Just return.
	return NULL;

	
}
