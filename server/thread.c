/**
 * @file thread.c
 * @purpose Implementation of thread control
 */

#include "thread.h"
#include "options.h"

pthread_mutex_t mutex_runstate = PTHREAD_MUTEX_INITIALIZER;
Runstate runstate = RUNNING;

/**
 * Set the runstate, causing all threads to exit when they next check Thread_Runstate
 * Repeated calls to this function have no effect on the runstate.
 * @param error - Set to true to indicate an error occured
 */
void Thread_QuitProgram(bool error)
{
	if (runstate != RUNNING)
	{
		Log(LOGNOTE, "Called when program is not running; runstate = %d", runstate);
		return;
	}


	Log(LOGNOTE, "Program will quit; error = %d", (int)error);

	//CRITICAL SECTION - We do NOT want multiple threads editing the runstate at the same time!
	pthread_mutex_lock(&mutex_runstate);
	if (error)
		runstate = QUIT_ERROR;
	else
		runstate = QUIT;
	
	gettimeofday(&g_options.end_time, NULL);
	pthread_mutex_unlock(&mutex_runstate);
	// End critical section
}

/**
 * Check the runstate; to be called periodically by all threads.
 * This function will call Thread_QuitProgram and change the Runstate there is an exit condition detected.
 */
Runstate Thread_Runstate()
{
	//TODO: Add real exit conditions; for testing purposes, set a timeout
	/*
	struct timeval time;
	gettimeofday(&time, NULL);
	Log(LOGDEBUG, "It has been %d seconds since program started.", time.tv_sec - g_options.start_time.tv_sec);
	if (time.tv_sec - g_options.start_time.tv_sec > 3)
	{
		Thread_QuitProgram(false);
	}
	*/
	
	// Just reading the runstate doesn't really require a mutex
	// The worst case: Another thread alters the runstate before this thread gets the result; this thread thinks the program is still running
	// In that case, the thread will run an extra cycle of its loop and *then* exit. Since the runstate can only be changed once.
	// We could put a mutex here anyway, but it will have an impact on how fast the loops can run.
	return runstate;
}
