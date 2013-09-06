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
	STATUS_UNAUTHORIZED = -2
} StatusCodes;

typedef struct FCGIContext FCGIContext;
typedef void (*ModuleHandler) (FCGIContext *context, char *params);

extern void FCGI_BeginControl(FCGIContext *context, bool force);
extern void FCGI_EndControl(FCGIContext *context);
extern bool FCGI_HasControl(FCGIContext *context, const char *key);
extern char *FCGI_KeyPair(char *in, const char **key, const char **value);
extern void FCGI_BeginJSON(FCGIContext *context, StatusCodes status_code);
extern void FCGI_JSONPair(const char *key, const char *value);
extern void FCGI_JSONLong(const char *key, long value);
extern void FCGI_JSONDouble(const char *key, double value);
extern void FCGI_JSONBool(const char *key, bool value);
extern void FCGI_JSONKey(const char *key);
extern void FCGI_PrintRaw(const char *format, ...);
extern void FCGI_EndJSON();
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


