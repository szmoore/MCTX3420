/**
 * @file fastcgi.h
 * @purpose Headers for the fastcgi web interface
 */
 
#ifndef _FASTCGI_H
#define _FASTCGI_H
 
/**(HTTP) Status codes that fcgi module handlers can return**/
typedef enum StatusCodes {
	STATUS_OK = 200,
	STATUS_ERROR = 400,
	STATUS_UNAUTHORIZED = 401
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
extern void FCGI_JSONValue(const char *format, ...);
extern void FCGI_EndJSON();
extern void FCGI_RejectJSON(FCGIContext *context);
extern void FCGI_RejectJSONEx(FCGIContext *context, StatusCodes status, const char *description);
extern void * FCGI_RequestLoop (void *data);

#endif


