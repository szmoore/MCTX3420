/**
 * @file main.c
 * @purpose Entry point to the program, starts threads, handles cleanup on program exit
 */

// --- Standard headers --- //
#include <stdlib.h>
#include <stdio.h>
#include <signal.h> // for signal handling

// --- Custom headers --- //
#include "options.h"

// --- Variable definitions --- //
Options g_options; // options passed to program through command line arguments

// --- Function definitions --- //

/**
 * @funct ParseArguments
 * @purpose Parse command line arguments, set up an options variable
 * @param argc - Num args
 * @param argv - Array of args
 * @param opts - Pointer to options.  &g_options
 */
void ParseArguments(int argc, char ** argv, Options * opts)
{
	options.program = argv[0]; // program name
	options.verbosity = LOGDEBUG; // default log level
	if (argc > 1)
		options.port = atoi(argv[1]); // Allow us change the port for testing (I keep getting "address in use" errors)
	else
		options.port = 8080; // Using 8080 instead of 80 for now because to use 80 you have to run the program as root

	Log(LOGDEBUG, "ParseArguments", "Called as %s with %d arguments.", options.program, argc);
}

/**
 * @funct SignalHandler
 * @purpose Handle signals
 * @param sig - The signal
 */
void SignalHandler(int sig)
{
	// At the moment just always exit.
	// Call `exit` so that Cleanup will be called to... clean up.
	Log(LOGWARN, "SignalHandler", "Got signal %d (%s). Exiting.", sig, strsignal(sig));
	exit(sig);
}

/**
 * @funct Cleanup
 * @purpose Called when program exits
 */
void Cleanup()
{
	Log(LOGDEBUG, "Cleanup", "Begin cleanup.");
	Log(LOGDEBUG, "Cleanup", "Finish cleanup.");

}


