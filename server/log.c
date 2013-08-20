/**
 * @file log.c
 * @purpose Implement logging and error handling functions
 */


#include <unistd.h>
#include <stdarg.h>

// --- Custom headers --- //
#include "common.h"
#include "log.h"
#include "options.h"

// --- Static variables --- //
static const char * unspecified_funct = "???";

// --- Function implementations --- //

/**
 * Print a message to stderr
 * @param level - Specify how severe the message is.
	If level is higher (less urgent) than the program's verbosity (see options.h) no message will be printed
 * @param funct - String indicating the function name from which this function was called.
	If this is NULL, Log will show the unspecified_funct string instead
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void LogEx(int level, const char * funct, ...)
{
	const char *fmt;
	va_list va;
	va_start(va, funct);
	fmt = va_arg(va, const char*);
	
	if (fmt == NULL) // sanity check
		FatalEx("Log", "Format string is NULL");

	// Don't print the message unless we need to
	if (level > g_options.verbosity)
		return;

	if (funct == NULL)
		funct = unspecified_funct;

	// Make a human readable severity string
	const char *severity;
	switch (level)
	{
		case LOGERR:
			severity = "ERROR";
			break;
		case LOGWARN:
			severity = "WARNING";
			break;
		case LOGNOTE:
			severity = "NOTICE";
			break;
		case LOGINFO:
			severity = "INFO";
			break;
		default:
			severity = "DEBUG";
			break;
	}

	// Print: Program name, PID, severity string, function name first
	fprintf(stderr, "%s [%d] : %s : %s - ", g_options.program, getpid(), severity, funct);

	// Then pass additional arguments with the format string to vfprintf for printing
	vfprintf(stderr, fmt, va);
	va_end(va);

	// End log messages with a newline
	fprintf(stderr, "\n");
}

/**
 * Handle a Fatal error in the program by printing a message and exiting the program
 *	CALLING THIS FUNCTION WILL CAUSE THE PROGAM TO EXIT
 * @param funct - Name of the calling function
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void FatalEx(const char * funct, ...)
{
	const char *fmt;
	va_list va;
	va_start(va, funct);
	fmt = va_arg(va, const char*);
	
	if (fmt == NULL)
	{
		// Fatal error in the Fatal function.
		// (This really shouldn't happen unless someone does something insanely stupid)
		FatalEx("Fatal", "Format string is NULL");
		return; // Should never get here
	}

	if (funct == NULL)
		funct = unspecified_funct;

	fprintf(stderr, "%s [%d] : %s : FATAL - ", g_options.program, getpid(), funct);

	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}


