/**
 * @file fastcgi.c
 * @brief Runs the FCGI request loop to handle web interface requests.
 *
 * fcgi_stdio.h must be included before all else so the stdio function
 * redirection works ok.
 */

#include <fcgi_stdio.h>
#include <openssl/sha.h>
#include <stdarg.h>

#include "common.h"
#include "sensor.h"
#include "actuator.h"
#include "control.h"
#include "options.h"
#include "image.h"
#include "pin_test.h"
#include "login.h"

/**The time period (in seconds) before the control key expires */
#define CONTROL_TIMEOUT 180



/**
 * Identifies build information and the current API version to the user.
 * Also useful for testing that the API is running and identifying the 
 * sensors and actuators present.
 * @param context The context to work in
 * @param params User specified paramters: [actuators, sensors]
 */ 
static void IdentifyHandler(FCGIContext *context, char *params) {
	bool ident_sensors = false, ident_actuators = false;
	bool has_control = FCGI_HasControl(context, getenv("COOKIE_STRING"));
	int i;

	FCGIValue values[2] = {{"sensors", &ident_sensors, FCGI_BOOL_T},
					 {"actuators", &ident_actuators, FCGI_BOOL_T}};
	if (!FCGI_ParseRequest(context, params, values, 2))
		return;

	FCGI_BeginJSON(context, STATUS_OK);
	FCGI_JSONPair("description", "MCTX3420 Server API (2013)");
	FCGI_JSONPair("build_date", __DATE__ " " __TIME__);
	FCGI_JSONLong("api_version", API_VERSION);
	FCGI_JSONBool("logged_in", has_control);
	FCGI_JSONPair("friendly_name", has_control ? context->friendly_name : "");

	//Sensor and actuator information
	if (ident_sensors) {
		FCGI_JSONKey("sensors");
		FCGI_JSONValue("{\n\t\t");
		for (i = 0; i < g_num_sensors; i++) {
			if (i > 0) {
				FCGI_JSONValue(",\n\t\t");
			}
			FCGI_JSONValue("\"%d\" : \"%s\"", i, Sensor_GetName(i)); 
		}
		FCGI_JSONValue("\n\t}");
	}
	if (ident_actuators) {
		FCGI_JSONKey("actuators");
		FCGI_JSONValue("{\n\t\t");
		for (i = 0; i < g_num_actuators; i++) {
			if (i > 0) {
				FCGI_JSONValue(",\n\t\t");
			}
			FCGI_JSONValue("\"%d\" : \"%s\"", i, Actuator_GetName(i)); 
		}
		FCGI_JSONValue("\n\t}");
	}
	FCGI_EndJSON();
}

/**
 * Gives the user a key that determines who has control over
 * the system at any one time. The key can be forcibly generated, revoking
 * any previous control keys. To be used in conjunction with HTTP 
 * basic authentication.
 * @param context The context to work in
 * @param force Whether to force key generation or not.
 * @return true on success, false otherwise (eg someone else already in control)
 */
bool FCGI_LockControl(FCGIContext *context, bool force) {
	time_t now = time(NULL);
	bool expired = now - context->control_timestamp > CONTROL_TIMEOUT;

	if (force || !*(context->control_key) || expired) 
	{
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
		return true;
	}
	return false;
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
			key != NULL && context->control_key[0] != '\0' &&
			!strcmp(context->control_key, key);
	if (result) {
		context->control_timestamp = now; //Update the control_timestamp
	}
	return result;
}


/**
 * Revokes the current control key, if present.
 * @param context The context to work in
 */
void FCGI_ReleaseControl(FCGIContext *context) {
	*(context->control_key) = 0;
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
 * Aids in parsing request parameters. 
 * Input: The expected keys along with their type and whether or not
 * they're required.
 * @param context The context to work in
 * @param params The parameter string to be parsed
 * @param values An array of FCGIValue's that specify expected keys
 * @param count The number of elements in 'values'.
 * @return true If the parameter string was parsed successfully, false otherwise.
 *         Modes of failure include: Invalid a parsing error on the value,
 *                                   an unknown key is specified,
 *                                   a key/value pair is specified more than once, or
 *                                   not all required keys were present.
 *         If this function returns false, it is guaranteed that FCGI_RejectJSON
 *         has already been called with the appropriate description message.
 */
bool FCGI_ParseRequest(FCGIContext *context, char *params, FCGIValue values[], size_t count)
{
	const char *key, *value;
	char buf[BUFSIZ], *ptr;
	size_t i;
	
	while ((params = FCGI_KeyPair(params, &key, &value))) {
		for (i = 0; i < count; i++) {
			if (!strcmp(key, values[i].key)) {
				FCGIValue *val = &values[i];

				if (FCGI_RECEIVED(val->flags)) {
					snprintf(buf, BUFSIZ, "Value already specified for '%s'.", key);
					FCGI_RejectJSON(context, buf);
					return false;
				}
				val->flags |= FCGI_PARAM_RECEIVED;

				switch(FCGI_TYPE(val->flags)) {
					case FCGI_BOOL_T:
						if (!*value) //No value: Default true
							*((bool*) val->value) = true;
						else {
							*((bool*) val->value) = !!(strtol(value, &ptr, 10));
							if (*ptr) {
								snprintf(buf, BUFSIZ, "Expected bool for '%s' but got '%s'", key, value);
								FCGI_RejectJSON(context, buf);
								return false;
							}
						}
						break;
					case FCGI_INT_T: case FCGI_LONG_T: {
						long parsed = strtol(value, &ptr, 10);
						if (!*value || *ptr) {
							snprintf(buf, BUFSIZ, "Expected int for '%s' but got '%s'", key, value);
							FCGI_RejectJSON(context, buf);
							return false;
						}

						if (FCGI_TYPE(val->flags) == FCGI_INT_T)
							*((int*) val->value) = (int) parsed;
						else
							*((long*) val->value) = parsed;
					}	break;
					case FCGI_DOUBLE_T:
						*((double*) val->value) = strtod(value, &ptr);
						if (!*value || *ptr) {
							snprintf(buf, BUFSIZ, "Expected float for '%s' but got '%s'", key, value);
							FCGI_RejectJSON(context, buf);
							return false;
						}
						break;
					case FCGI_STRING_T:
						*((const char**) val->value) = value;
						break;
					default:
						Fatal("Invalid type %d given", FCGI_TYPE(val->flags));
				}
				break; //No need to search any more
			}
		} //End for loop
		if (i == count) {
			snprintf(buf, BUFSIZ, "Unknown key '%s' specified", key);
			FCGI_RejectJSON(context, buf);
			return false;
		}
	}

	//Check that required parameters are received
	for (i = 0; i < count; i++) {
		if (FCGI_IS_REQUIRED(values[i].flags) && !FCGI_RECEIVED(values[i].flags)) {
			snprintf(buf, BUFSIZ, "Key '%s' required, but was not given.", values[i].key);
			FCGI_RejectJSON(context, buf);
			return false;
		}
	}
	return true;
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
	//Time and running statistics
	struct timeval now;
	gettimeofday(&now, NULL);
	FCGI_JSONDouble("start_time", TIMEVAL_TO_DOUBLE(g_options.start_time));
	FCGI_JSONDouble("current_time", TIMEVAL_TO_DOUBLE(now));
	FCGI_JSONDouble("running_time", TIMEVAL_DIFF(now, g_options.start_time));
	FCGI_JSONPair("control_state", Control_GetModeName());
}

/**
 * Generic accept response in JSON format.
 * @param context The context to work in
 * @param description A short description.
 * @param cookie Optional. If given, the cookie field is set to that value.
 */
void FCGI_AcceptJSON(FCGIContext *context, const char *description, const char *cookie)
{
	printf("Content-type: application/json; charset=utf-8\r\n");
	if (cookie) {
		printf("Set-Cookie: %s\r\n", cookie);
	}
	printf("\r\n{\r\n");
	printf("\t\"module\" : \"%s\"", context->current_module);
	FCGI_JSONLong("status", STATUS_OK);
	FCGI_JSONPair("description", description);
	FCGI_EndJSON();
}

/**
 * Adds a key/value pair to a JSON response. The response must have already
 * been initiated by FCGI_BeginJSON. Special characters are not escaped.
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
 * To be used when the input parameters are rejected. The return data
 * will also have debugging information provided.
 * @param context The context to work in
 * @param status The status the return data should have.
 * @param description A short description of why the input was rejected.
 */
void FCGI_RejectJSONEx(FCGIContext *context, StatusCodes status, const char *description)
{
	if (description == NULL)
		description = "Unknown";
	
	Log(LOGINFO, "%s: Rejected query with: %d: %s", context->current_module, status, description);
	FCGI_BeginJSON(context, status);
	FCGI_JSONPair("description", description);
	FCGI_JSONLong("responsenumber", context->response_number);
	//FCGI_JSONPair("params", getenv("QUERY_STRING")); //A bad idea if contains password but also if contains unescaped stuff
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
 * Write binary data
 * See fwrite
 */
void FCGI_WriteBinary(void * data, size_t size, size_t num_elem)
{
	Log(LOGDEBUG,"Writing!");
	fwrite(data, size, num_elem, stdout);
}

/**
 * Escapes a string so it can be used safely.
 * Currently escapes to ensure the validity for use as a JSON string
 * Does not support unicode specifiers in the form of \uXXXX.
 * @param buf The string to be escaped
 * @return The escaped string (return value == buf)
 */
char *FCGI_EscapeText(char *buf)
{
	int length, i;
	length = strlen(buf);
	
	//Escape special characters. Must count down to escape properly
	for (i = length - 1; i >= 0; i--) {
		if (buf[i] < 0x20) { //Control characters
			buf[i] = ' ';
		} else if (buf[i] == '"') {
			if (i-1 >= 0 && buf[i-1] == '\\') 
				i--;
			else
				buf[i] = '\'';
		} else if (buf[i] == '\\') {
			if (i-1 >= 0 && buf[i-1] == '\'')
				i--;
			else
				buf[i] = ' ';
		}
	}
	return buf;
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
	
	Log(LOGDEBUG, "Start loop");
	while (FCGI_Accept() >= 0) {
		
		ModuleHandler module_handler = NULL;
		char module[BUFSIZ], params[BUFSIZ];
		//Don't need to copy if we're not modifying string contents
		const char *cookie = getenv("COOKIE_STRING");
		
		//strncpy doesn't zero-truncate properly
		snprintf(module, BUFSIZ, "%s", getenv("DOCUMENT_URI_LOCAL"));
		snprintf(params, BUFSIZ, "%s", getenv("QUERY_STRING"));

		Log(LOGDEBUG, "Got request #%d - Module %s, params %s", context.response_number, module, params);
		Log(LOGDEBUG, "Cookie: %s", cookie);

		
		//Remove trailing slashes (if present) from module query
		size_t lastchar = strlen(module) - 1;
		if (lastchar > 0 && module[lastchar] == '/')
			module[lastchar] = 0;

		//Default to the 'identify' module if none specified
		if (!*module) 
			strcpy(module, "identify");
		
		if (!strcmp("identify", module)) {
			module_handler = IdentifyHandler;
		} else if (!strcmp("control", module)) {
			module_handler = Control_Handler;
		} else if (!strcmp("sensors", module)) {
			module_handler = Sensor_Handler;
		} else if (!strcmp("actuators", module)) {
			module_handler = Actuator_Handler;
		} else if (!strcmp("image", module)) {
			module_handler = Image_Handler;
		} else if (!strcmp("pin", module)) { 
			module_handler = Pin_Handler; // *Debug only* pin test module
		} else if (!strcmp("bind", module)) {
			module_handler = Login_Handler;
		} else if (!strcmp("unbind", module)) {
			module_handler = Logout_Handler;
		}

		context.current_module = module;
		context.response_number++;
		
		if (module_handler) 
		{
			//if (module_handler != Login_Handler && module_handler != IdentifyHandler)
			if (false) // Testing
			{
				if (!FCGI_HasControl(&context, cookie))
				{
					FCGI_RejectJSON(&context, "Please login. Invalid control key.");
					continue;	
				}

				//Escape all special characters.
				//Don't escape for login (password may have special chars?)
				FCGI_EscapeText(params);
			}

			module_handler(&context, params);
		} 
		else 
		{
			FCGI_RejectJSON(&context, "Unhandled module");
		}
	}

	Log(LOGDEBUG, "Thread exiting.");
	// NOTE: Don't call pthread_exit, because this runs in the main thread. Just return.
	return NULL;
}
