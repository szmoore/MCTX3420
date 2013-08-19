/**
 * @file fastcgi.h
 * @purpose Headers for the fastcgi web interface
 */
 
#ifndef _FASTCGI_H
#define _FASTCGI_H
 
/**HTTP status codes that fcgi module handlers can return**/
typedef enum StatusCodes {
	STATUS_OK = 200, 
	STATUS_BADREQUEST = 400,
	STATUS_UNAUTHORIZED = 401
} StatusCodes;

typedef void (*ModuleHandler) (void *data, char *params);

extern char *FCGI_KeyPair(char *in, const char **key, const char **value);
extern void FCGI_BeginJSON(StatusCodes status_code, const char *module);
extern void FCGI_BuildJSON(const char *key, const char *value);
extern void FCGI_EndJSON();
extern void FCGI_RequestLoop (void *data);

#define SENSOR_QUERYBUFSIZ 10

#endif //_FASTCGI_H
