/**
 * @file main.c
 * @brief main and its helper functions, signal handling and cleanup functions
 */

// --- Custom headers --- //
#include "common.h"
#include "options.h"
#include "sensor.h"
#include "actuator.h"
#include "control.h"
#include "pin_test.h"
#include "bbb_pin_defines.h"

// --- Standard headers --- //
#include <syslog.h> // for system logging
#include <signal.h> // for signal handling


// --- Variable definitions --- //
Options g_options; // options passed to program through command line arguments

// --- Function definitions --- //

/**
 * Parse command line arguments, initialise g_options
 * @param argc - Number of arguments
 * @param argv - Array of argument strings
 */
void ParseArguments(int argc, char ** argv)
{


	g_options.program = argv[0]; // program name
	g_options.verbosity = LOGDEBUG; // default log level
	gettimeofday(&(g_options.start_time), NULL); // Start time


	g_options.auth_method = AUTH_NONE;  // Don't use authentication
	g_options.auth_uri = ""; // 
	g_options.ldap_base_dn = "";
	


	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-')
			Fatal("Unexpected argv[%d] - %s", i, argv[i]);

		if (i+1 >= argc || argv[i+1][0] == '-')
			Fatal("No argument following switch %s", argv[i]);
		
		if (strlen(argv[i]) > 2)
			Fatal("Human readable switches are not supported.");

		char * end = NULL;
		switch (argv[i][1])
		{
			// Set program verbosity
			case 'v':
				g_options.verbosity = strtol(argv[++i], &end, 10);
				break;
			// Enable/Disable pin test
			case 'p':
				g_options.enable_pin = !(strtol(argv[++i], &end, 10));
				break;
			// LDAP URI
			case 'A':
				g_options.auth_uri = argv[++i];
				break;
			// LDAP DN
			case 'd':
				g_options.ldap_base_dn = argv[++i];
				break;
			default:
				Fatal("Unrecognised switch %s", argv[i]);
				break;
		}

		if (end != NULL && *end != '\0')
			Fatal("argv[%d] -%c requires an integer (got \"%s\" instead)", i-1, argv[i-1][0], argv[i]);
	}	

	Log(LOGDEBUG, "Verbosity: %d", g_options.verbosity);
	Log(LOGDEBUG, "Pin Module Enabled: %d", g_options.enable_pin);
	Log(LOGDEBUG, "Auth URI: %s", g_options.auth_uri);
	Log(LOGDEBUG, "LDAP Base DN: %s", g_options.ldap_base_dn);

	if (g_options.auth_uri[0] != '\0')
	{
		//HACK...
		if (PathExists(g_options.auth_uri))
			g_options.auth_method = AUTH_SHADOW;
		else
			g_options.auth_method = AUTH_LDAP;
	}
	
}

/**
 * Cleanup before the program exits
 */
void Cleanup()
{
	Log(LOGDEBUG, "Begin cleanup.");
	Sensor_Cleanup();
	//Actuator_Cleanup();
	Log(LOGDEBUG, "Finish cleanup.");
}

/**
 * Main entry point; start worker threads, setup signal handling, wait for threads to exit, exit
 * @param argc - Num args
 * @param argv - Args
 * @returns 0 on success, error code on failure
 * NOTE: NEVER USE exit(3)! Instead call Thread_QuitProgram
 */
int main(int argc, char ** argv)
{
	// Open log before calling ParseArguments (since ParseArguments may call the Log functions)
	openlog("mctxserv", LOG_PID | LOG_PERROR, LOG_USER);
	Log(LOGINFO, "Server started");

	ParseArguments(argc, argv); // Setup the g_options structure from program arguments

	Sensor_Init();
	Actuator_Init();
	Pin_Init();
	
	// Try and start things
	const char *ret;
	if ((ret = Control_SetMode(CONTROL_START, "test")) != NULL)
		Fatal("Control_SetMode failed with '%s'", ret);

	// run request thread in the main thread
	FCGI_RequestLoop(NULL);

	if ((ret = Control_SetMode(CONTROL_STOP, "test")) != NULL)
		Fatal("Control_SetMode failed with '%s'", ret);
	//Sensor_StopAll();
	//Actuator_StopAll();

	Pin_Close();

	Cleanup();
	return 0;
}


