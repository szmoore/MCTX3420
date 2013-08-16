/**
 * @file main.c
 * @purpose main and its helper functions, signal handling and cleanup functions
 */
#include "common.h"


// --- Standard headers --- //
#include <signal.h> // for signal handling

// --- Custom headers --- //
#include "query.h"
#include "log.h"
#include "options.h"
#include "sensor.h"

// --- Variable definitions --- //
Options g_options; // options passed to program through command line arguments
Sensor g_sensors[NUMSENSORS]; // sensors array

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
//TODO: Something that gets massively annoying with threads is that you can't predict which one gets the signal
// There are ways to deal with this, but I can't remember them
void SignalHandler(int signal)
{
	// At the moment just always exit.
	// Call `exit` so that Cleanup will be called to... clean up.
	Log(LOGWARN, "Got signal %d (%s). Exiting.", signal, strsignal(signal));
	exit(signal);
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
	ParseArguments(argc, argv);

	// start sensor threads
	for (int i = 0; i < NUMSENSORS; ++i)
	{
		Sensor_Init(g_sensors+i, i);
		pthread_create(&(g_sensors[i].thread), NULL, Sensor_Main, (void*)(g_sensors+i));
	}

	// run request thread in the main thread
	Query_Main(NULL); //TODO: Replace with FastCGI code
	return 0;
}


