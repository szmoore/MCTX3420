/**
 * @file log.h
 * @purpose Declaration of functions for printing log messages and/or terminating program after a fatal error
 */

#ifndef _LOG_H
#define _LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>


// An enum to make the severity of log messages human readable in code
enum {LOGERR=0, LOGWARN=1, LOGNOTE=2, LOGINFO=3,LOGDEBUG=4};

extern void Log(int level, char * funct, char * fmt,...); // General function for printing log messages to stderr
extern void Fatal(char * funct, char * fmt, ...); // Function that deals with a fatal error (prints a message, then exits the program).

#endif //_LOG_H

//EOF
