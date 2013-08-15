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

} Options;

/** The only instance of the Options struct **/
extern Options g_options;

#endif //_OPTIONS_H
