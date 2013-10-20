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
	struct timespec start_time;
	/** Time at which program exits **/
	struct timespec end_time;

	/** Whether or not to enable the pin_test module **/
	bool enable_pin;
	
	/** URI for authentication **/
	const char * auth_uri;

	/** Additional options for authentication (to be parsed in Login_Handler) **/
	const char * auth_options;

	/** Authentication method **/
	enum {AUTH_NONE, AUTH_LDAP, AUTH_SHADOW, AUTH_MYSQL} auth_method;

	/** Experiments directory **/
	const char *experiment_dir;
} Options;

/** The only instance of the Options struct **/
extern Options g_options;

#endif //_OPTIONS_H
