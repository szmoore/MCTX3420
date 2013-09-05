/**
 * @file fastcgi.c
 * @brief Runs the FCGI request loop to handle web interface requests.
 *
 * fcgi_stdio.h must be included before all else so the stdio function
 * redirection works ok.
 */

#include <fcgi_stdio.h>
#include <openssl/sha.h>
#include <time.h>

#include "common.h"
#include "sensor.h"
#include "control.h"
#include "options.h"

/**The time period (in seconds) before the control key expires @ */
#define CONTROL_TIMEOUT 180

/**Contextual information related to FCGI requests*/
struct FCGIContext {
	/**The time of last valid user access possessing the control key*/
	time_t control_timestamp;
	char control_key[41];
	char control_ip[16];
	/**The name of the current module**/
	const char *current_module;
	/**For debugging purposes?**/
	int response_number;
};

/**
 * Identifies current version info. Useful for testing that the API is running.
 * TODO - Consider adding info about available sensors and actuators (eg capabilities)?
 */ 
static void IdentifyHandler(FCGIContext *context, char *params) {
	FCGI_BeginJSON(context, STATUS_OK);
	FCGI_JSONPair("description", "MCTX3420 Server API (2013)");
	FCGI_JSONPair("build_date", __DATE__ " " __TIME__);
	FCGI_EndJSON();
}

/**
 * Gives the user a key that determines who has control over
 * the system at any one time. The key can be forcibly generated, revoking
 * any previous control keys. To be used in conjunction with HTTP 
 * basic authentication.
 * This function will generate a JSON response that indicates success/failure.
 * @param context The context to work in
 * @param force Whether to force key generation or not.
 */ 
void FCGI_BeginControl(FCGIContext *context, bool force) {
	time_t now = time(NULL);
	bool expired = now - context->control_timestamp > CONTROL_TIMEOUT;
	
	if (force || !*(context->control_key) || expired) {
		SHA_CTX sha1ctx;
		unsigned char sha1[20];
		int i = rand();

		SHA1_Init(&sha1ctx);
		SHA1_Update(&sha1ctx, &now, sizeof(now));
		SHA1_Update(&sha1ctx, &i, sizeof(i));
		SHA1_Final(sha1, &sha1ctx);

		context->control_timestamp = now;
		for (i = 0; i < 20; i++)
			sprintf(context->control_key + i * 2, "%02x", sha1[i]);
		snprintf(context->control_ip, 16, "%s", getenv("REMOTE_ADDR"));
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("key", context->control_key);
		FCGI_EndJSON();		
	} else {
		char buf[128];
		strftime(buf, 128, "%H:%M:%S %d-%m-%Y",
			localtime(&(context->control_timestamp))); 
		FCGI_BeginJSON(context, STATUS_UNAUTHORIZED);
		FCGI_JSONPair("description", "Another user already has control");
		FCGI_JSONPair("current_user", context->control_ip); 
		FCGI_JSONPair("when", buf);
		FCGI_EndJSON();
	}
}

/**
 * Given an FCGIContext, determines if the current user (as specified by
 * the key) has control or not. If validated, the context control_timestamp is
 * updated.
 * @param context The context to work in
 * @param key The control key to be validated.
 * @return TRUE if authorized, FALSE if not.
 */
bool FCGI_HasControl(FCGIContext *context, const char *key) {
	time_t now = time(NULL);
	int result = (now - context->control_timestamp) <= CONTROL_TIMEOUT &&
				 key != NULL && !strcmp(context->control_key, key);
	if (result) {
		context->control_timestamp = now; //Update the control_timestamp
	}
	return result;
}


/**
 * Revokes the current control key, if present.
 * @param context The context to work in
 */
void FCGI_EndControl(FCGIContext *context) {
	*(context->control_key) = 0;
	FCGI_BeginJSON(context, STATUS_OK);
	FCGI_EndJSON();
	return;
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
 * @param value The value associated with the key.
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
 * Similar to FCGI_JsonPair except for boolean values.
 * @param key The key of the JSON entry
 * @param value The value associated with the key
 */
void FCGI_JSONBool(const char *key, bool value)
{
	printf(",\r\n\t\"%s\" : %s", key, value ? "true" : "false");
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
 * Ends a JSON response that was initiated by FCGI_BeginJSON.
 */
void FCGI_EndJSON() 
{
	printf("\r\n}\r\n");
}

/**
 * To be used when the input parameters are invalid. The return data will
 * have a status of STATUS_ERROR, along with other debugging information.
 * @param context The context to work in
 */
void FCGI_RejectJSON(FCGIContext *context)
{
	FCGI_RejectJSONEx(context, STATUS_ERROR, "Invalid request");
}

/**
 * To be used when the input parameters are rejected. The return data
 * will also have debugging information provided.
 * @param context The context to work in
 * @param status The status the return data should have.
 * @param description A short description of why the input was rejected.
 */
void FCGI_RejectJSONEx(FCGIContext *context, StatusCodes status, const char *description)
{
	description = !description ? "" : description;
	
	Log(LOGINFO, "%s: Rejected query with: %d: %s", context->current_module, status, description);
	FCGI_BeginJSON(context, status);
	FCGI_JSONPair("description", description);
	FCGI_JSONLong("responsenumber", context->response_number);
	FCGI_JSONPair("params", getenv("QUERY_STRING"));
	FCGI_JSONPair("host", getenv("SERVER_HOSTNAME"));
	FCGI_JSONPair("user", getenv("REMOTE_USER"));
	FCGI_JSONPair("ip", getenv("REMOTE_ADDR"));
	FCGI_EndJSON();
}

/**
 * Generates a response to the client as described by the format parameter and
 * extra arguments (exactly like printf). To be used when none of the other
 * predefined functions will work exactly as needed. Extra care should be taken
 * to ensure the correctness of the output.
 * @param format The format string
 * @param ... Any extra arguments as required by the format string.
 */
void FCGI_PrintRaw(const char *format, ...)
{
	va_list list;
	va_start(list, format);
	vprintf(format, list);
	va_end(list);
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
		
		if (!*module || !strcmp("identify", module)) {
			module_handler = IdentifyHandler;
		} else if (!strcmp("control", module)) {
			module_handler = Control_Handler;
		} else if (!strcmp("sensors", module)) {
			module_handler = Sensor_Handler;
		}

		context.current_module = module;
		if (module_handler) {
			module_handler(&context, params);
		} else {
			strncat(module, " (unhandled)", BUFSIZ);
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
