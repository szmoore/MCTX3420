/**
 * @file main.c
 * @purpose Entry point to the program, starts threads, handles cleanup on program exit
 */

#define _POSIX_C_SOURCE 200809L // For strsignal to work

// --- Standard headers --- //
#include <stdlib.h>
#include <stdio.h>
#include <signal.h> // for signal handling
#include <string.h> // string functions

// --- Custom headers --- //
#include "log.h"
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
	opts->program = argv[0]; // program name
	opts->verbosity = LOGDEBUG; // default log level
	Log(LOGDEBUG, "Called as %s with %d arguments.", opts->program, argc);
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
	Log(LOGWARN, "Got signal %d (%s). Exiting.", sig, strsignal(sig));
	exit(sig);
}

/**
 * @funct Cleanup
 * @purpose Called when program exits
 */
void Cleanup()
{
	Log(LOGDEBUG, "Begin cleanup.");
	Log(LOGDEBUG, "Finish cleanup.");

}

/**
 * @funct main
 * @purpose Main entry point; start worker threads, setup signal handling, wait for threads to exit, exit
 * @param argc - Num args
 * @param argv - Args
 * @returns 0 on success, error code on failure
 */
int main(int argc, char ** argv)
{
	ParseArguments(argc, argv, &g_options);
	return 0;
}


