/**
 * @file log.c
 * @purpose Implement logging and error handling functions
 */


#include <unistd.h>

// --- Custom headers --- //
#include "log.h"
#include "options.h"

// --- Static variables --- //
static char * unspecified_funct = (char*)"???";

// --- Function implementations --- //

/**
 * @funct Log
 * @purpose Print a message to stderr
 * @param level - Specify how severe the message is.
	If level is higher (less urgent) than the program's verbosity (see options.h) no message will be printed
 * @param funct - String indicating the function name from which this function was called.
	If this is NULL, Log will show the unspecified_funct string instead
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void Log(int level, char * funct, char * fmt, ...)
{
	if (fmt == NULL) // sanity check
		Fatal("Log", "Format string is NULL");

	// Don't print the message unless we need to
	if (level > options.verbosity) 
		return;

	if (funct == NULL)
		funct = unspecified_funct;

	// Make a human readable severity string
	char severity[BUFSIZ];
	switch (level)
	{
		case LOGERR:
			sprintf(severity, "ERROR");
			break;
		case LOGWARN:
			sprintf(severity, "WARNING");
			break;
		case LOGNOTE:
			sprintf(severity, "NOTICE");
			break;
		case LOGINFO:
			sprintf(severity, "INFO");
			break;
		default:
			sprintf(severity, "DEBUG");
			break;
	}

	// Print: Program name, PID, severity string, function name first
	fprintf(stderr, "%s [%d] : %s : %s - ", options.program, getpid(), severity, funct);

	// Then pass additional arguments with the format string to vfprintf for printing
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);

	// End log messages with a newline
	fprintf(stderr, "\n");
}

/**
 * @funct Fatal
 * @purpose Handle a Fatal error in the program by printing a message and exiting the program
	CALLING THIS FUNCTION WILL CAUSE THE PROGAM TO EXIT
 * @param funct - Name of the calling function
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void Fatal(char * funct, char * fmt, ...)
{
	
	if (fmt == NULL)
	{
		// Fatal error in the Fatal function.
		// (This really shouldn't happen unless someone does something insanely stupid)
		Fatal("Fatal", "Format string is NULL");
		return; // Should never get here
	}

	if (funct == NULL)
		funct = unspecified_funct;

	fprintf(stderr, "%s [%d] : %s : FATAL - ", options.program, getpid(), funct);

	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}


