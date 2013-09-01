/**
 * @file fastcgi.h
 * @purpose Headers for the fastcgi web interface
 */
 
#ifndef _FASTCGI_H
#define _FASTCGI_H
 
/**Status codes that fcgi module handlers can return**/
typedef enum StatusCodes {
	STATUS_OK = 0,
	STATUS_ERROR = -1,
	STATUS_UNAUTHORIZED = -2
} StatusCodes;

typedef struct FCGIContext FCGIContext;
typedef void (*ModuleHandler) (FCGIContext *data, char *params);

extern bool FCGI_Authorized(FCGIContext *context, const char *key);
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
extern void * FCGI_RequestLoop (void *data);

#endif


