/**
 * @file main.c
 * @purpose main and its helper functions, signal handling and cleanup functions
 */

// --- Custom headers --- //
#include "common.h"
#include "options.h"
#include "sensor.h"

// --- Standard headers --- //
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
	Log(LOGDEBUG, "Called as %s with %d arguments.", g_options.program, argc);
}

/**
 * Handle a signal
 * @param signal - The signal number
 */
//TODO: Something that gets massively annoying with threads is that you can't predict which one gets the signal
// There are ways to deal with this, but I can't remember them
// Probably sufficient to just call Thread_QuitProgram here
void SignalHandler(int signal)
{
	// At the moment just always exit.
	// Call `exit` so that Cleanup will be called to... clean up.
	Log(LOGWARN, "Got signal %d (%s). Exiting.", signal, strsignal(signal));
	Thread_QuitProgram(false);
	//exit(signal);
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
 * NOTE: NEVER USE exit(3)! Instead call Thread_QuitProgram
 */
int main(int argc, char ** argv)
{
	ParseArguments(argc, argv);

	// signal handler
	//TODO: Make this work
	/*
	int signals[] = {SIGINT, SIGSEGV, SIGTERM};
	for (int i = 0; i < sizeof(signals)/sizeof(int); ++i)
	{
		signal(signals[i], SignalHandler);
	}
	*/
	Sensor_Spawn();

	// run request thread in the main thread
	FCGI_RequestLoop(NULL);

	// Join the dark side, Luke
	// *cough*
	// Join the sensor threads
	Sensor_Join();
	Cleanup();
	return 0;
}


