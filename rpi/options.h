/**
 * @file options.h
 * @purpose Declaration of structure to handle options passed to program
 */

#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct
{
	const char * program; //name of program
	int verbosity; // verbosity level

} Options;

extern Options g_options;

#endif //_OPTIONS_H
