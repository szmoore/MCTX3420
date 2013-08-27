/**
 * @file options.h
 * @purpose Define the Options structure and the g_options variable
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
	struct timeval start_time;
	/** Time at which program exits **/
	struct timeval end_time;

} Options;

/** The only instance of the Options struct **/
extern Options g_options;

#endif //_OPTIONS_H
