/**
 * @file fastcgi.h
 * @brief Headers for the fastcgi web interface
 */
 
#ifndef _FASTCGI_H
#define _FASTCGI_H
 
/**
 * Status codes that fcgi module handlers can return
 * Success status codes have values > 0
 * Failure status codes have values <(=) 0 
 * Note: 0 is counted as an error code to minimise confusion
 * with in-browser JSON parsing error codes
 */
typedef enum StatusCodes {
	STATUS_OK = 1,
	STATUS_ERROR = -1,
	STATUS_UNAUTHORIZED = -2,
	STATUS_NOTRUNNING = -3,
	STATUS_ALREADYEXISTS = -4
} StatusCodes;

#define FCGI_PARAM_REQUIRED (1 << 0)
#define FCGI_PARAM_RECEIVED (1 << 1)
#define FCGI_BOOL_T (1 << 2)
#define FCGI_INT_T	(1 << 3)
#define FCGI_LONG_T (1 << 4)
#define FCGI_DOUBLE_T (1 << 5)
#define FCGI_STRING_T (1 << 6)
#define FCGI_REQUIRED(x) ((x) | FCGI_PARAM_REQUIRED)
#define FCGI_IS_REQUIRED(x) ((x) & FCGI_PARAM_REQUIRED)
#define FCGI_RECEIVED(x) ((x) & FCGI_PARAM_RECEIVED)
#define FCGI_TYPE(x) ((x) & ~(FCGI_PARAM_REQUIRED | FCGI_PARAM_RECEIVED))

#define CONTROL_KEY_BUFSIZ 41

typedef struct FCGIValue {
	const char *key;
	void *value;
	unsigned flags;
} FCGIValue;

typedef enum {USER_UNAUTH, USER_NORMAL, USER_ADMIN} UserType;

/**Contextual information related to FCGI requests*/
typedef struct  
{
	/**The time of last valid user access possessing the control key**/
	time_t control_timestamp;
	/**A SHA-1 hash that is the control key, determining who is logged in**/
	char control_key[CONTROL_KEY_BUFSIZ]; 
	/**The IPv4 address of the logged-in user**/
	char control_ip[16];
	/**Determines if the user is an admin or not**/
	UserType user_type;
	/**Name of the logged in user**/
	char user_name[31];
	/**User directory for the logged in user**/
	char user_dir[BUFSIZ];
	/**The name of the current module**/
	const char *current_module;
	/**For debugging purposes?**/
	int response_number;
} FCGIContext;

typedef void (*ModuleHandler) (FCGIContext *context, char *params);

extern bool FCGI_LockControl(FCGIContext *context, const char * user_name, UserType user_type);
extern void FCGI_ReleaseControl(FCGIContext *context);
extern bool FCGI_HasControl(FCGIContext *context, const char *key);
extern char *FCGI_KeyPair(char *in, const char **key, const char **value);
extern bool FCGI_ParseRequest(FCGIContext *context, char *params, FCGIValue values[], size_t count);
extern void FCGI_BeginJSON(FCGIContext *context, StatusCodes status_code);
extern void FCGI_AcceptJSON(FCGIContext *context, const char *description, const char *cookie);
extern void FCGI_JSONPair(const char *key, const char *value);
extern void FCGI_JSONLong(const char *key, long value);
extern void FCGI_JSONDouble(const char *key, double value);
extern void FCGI_JSONBool(const char *key, bool value);
extern void FCGI_JSONKey(const char *key);
extern void FCGI_PrintRaw(const char *format, ...);
extern void FCGI_EndJSON();
extern void FCGI_RejectJSONEx(FCGIContext *context, StatusCodes status, const char *description);
extern char *FCGI_EscapeText(char *buf);
extern void *FCGI_RequestLoop (void *data);

extern void FCGI_WriteBinary(void * data, size_t size, size_t num_elem);

/**
 * Shortcut to calling FCGI_RejectJSONEx. Sets the error code
 * to STATUS_ERROR.
 * @param context The context to work in
 * @param description A short description of why the request was rejected.
 * @see FCGI_RejectJSONEx
 */
#define FCGI_RejectJSON(context, description) FCGI_RejectJSONEx(context, STATUS_ERROR, description)

/**
 * Custom formatting function for the JSON value. To be used in 
 * conjunction with FCGI_JSONKey. Care should be taken to ensure
 * that valid JSON is produced.
 * @see FCGI_PrintRaw for calling syntax
 */
#define FCGI_JSONValue FCGI_PrintRaw

#endif


