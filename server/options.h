/**
 * @file options.h
 * @brief Define the Options structure and the g_options variable
 */

#ifndef _OPTIONS_H
#define _OPTIONS_H


/** Store options passed or calculated from arguments to the program **/
typedef struct
{
	/** Name of program **/
	const char * program;
	/** Determines at what level log messages are shown **/
	int verbosity;
	/** Time at which program begins to run **/
	struct timeval start_time;
	/** Time at which program exits **/
	struct timeval end_time;

	/** Whether or not to enable the pin_test module **/
	bool enable_pin;
	
	/** URI for authentication **/
	const char * auth_uri;

	/** Base DN for LDAP authentication **/
	const char * ldap_base_dn;

	/** Authentication method **/
	enum {AUTH_NONE, AUTH_LDAP, AUTH_SHADOW} auth_method;

	

} Options;

/** The only instance of the Options struct **/
extern Options g_options;

#endif //_OPTIONS_H
