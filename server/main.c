/**
 * @file main.c
 * @purpose main and its helper functions, signal handling and cleanup functions
 */

#define _POSIX_C_SOURCE 200809L // For strsignal to work

// --- Standard headers --- //
#include <stdlib.h>
#include <stdio.h>
#include <signal.h> // for signal handling
#include <string.h> // string functions
#include <pthread.h>

// --- Custom headers --- //
#include "log.h"
#include "options.h"

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
	Log(LOGDEBUG, "Called as %s with %d arguments.", g_options.program, argc);
}

/**
 * Handle a signal
 * @param signal - The signal number
 */
void SignalHandler(int signal)
{
	// At the moment just always exit.
	// Call `exit` so that Cleanup will be called to... clean up.
	Log(LOGWARN, "Got signal %d (%s). Exiting.", sig, strsignal(sig));
	exit(sig);
}

/**
 * Cleanup before the program exits
 */
void Cleanup()
{
	Log(LOGDEBUG, "Begin cleanup.");
	Log(LOGDEBUG, "Finish cleanup.");

}

/**
 * Main entry point; start worker threads, setup signal handling, wait for threads to exit, exit
 * @param argc - Num args
 * @param argv - Args
 * @returns 0 on success, error code on failure
 */
int main(int argc, char ** argv)
{
	ParseArguments(argc, argv, &g_options);
	return 0;
}


