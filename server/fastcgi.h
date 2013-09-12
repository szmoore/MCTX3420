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
	STATUS_OUTOFRANGE = -3
} StatusCodes;

#define FCGI_PARAM_REQUIRED (1 << 0)
#define FCGI_PARAM_RECEIVED (1 << 1)
#define FCGI_BOOL_T (1 << 2)
#define FCGI_LONG_T (1 << 3)
#define FCGI_DOUBLE_T (1 << 4)
#define FCGI_STRING_T (1 << 5)
#define FCGI_REQUIRED(x) ((x) | FCGI_PARAM_REQUIRED)
#define FCGI_IS_REQUIRED(x) ((x) & FCGI_PARAM_REQUIRED)
#define FCGI_RECEIVED(x) ((x) & FCGI_PARAM_RECEIVED)
#define FCGI_TYPE(x) ((x) & ~(FCGI_PARAM_REQUIRED | FCGI_PARAM_RECEIVED))

typedef struct FCGIValue {
	const char *key;
	void *value;
	unsigned flags;
} FCGIValue;

typedef struct FCGIContext FCGIContext;
typedef void (*ModuleHandler) (FCGIContext *context, char *params);

extern void FCGI_BeginControl(FCGIContext *context, bool force);
extern void FCGI_EndControl(FCGIContext *context);
extern bool FCGI_HasControl(FCGIContext *context, const char *key);
extern char *FCGI_KeyPair(char *in, const char **key, const char **value);
extern bool FCGI_ParseRequest(FCGIContext *context, char *params, FCGIValue values[], size_t count);
extern void FCGI_BeginJSON(FCGIContext *context, StatusCodes status_code);
extern void FCGI_JSONPair(const char *key, const char *value);
extern void FCGI_JSONLong(const char *key, long value);
extern void FCGI_JSONDouble(const char *key, double value);
extern void FCGI_JSONBool(const char *key, bool value);
extern void FCGI_JSONKey(const char *key);
extern void FCGI_PrintRaw(const char *format, ...);
extern void FCGI_EndJSON();
extern char *FCGI_EscapeJSON(char *buf);
extern void FCGI_RejectJSONEx(FCGIContext *context, StatusCodes status, const char *description);
extern void *FCGI_RequestLoop (void *data);

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


