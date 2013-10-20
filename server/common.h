/**
 * @file common.h
 * @brief Common header includes
 */

#ifndef _COMMON_H
#define _COMMON_H

/** Defines required to allow various C standard functions to be used **/
#define _POSIX_C_SOURCE 200809L
//#define _BSD_SOURCE
#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
/** Determine if we're running on the BBB **/
#ifdef __arm__
	#define _BBB
#else
	//#warning This software was designed for the BeagleBone Black. Some features may not work.
#endif //__arm__

/** The current API version **/
#define API_VERSION 0

//#define REALTIME_VERSION



#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "log.h"
#include "fastcgi.h"
#include "control.h"

/**Converts a timeval to a double**/
#define TIMEVAL_TO_DOUBLE(tv) ((tv).tv_sec + 1e-9 * ((tv).tv_nsec))
/**Takes the tv1-tv2 between two timevals and returns the result as a double*/
#define TIMEVAL_DIFF(tv1, tv2) ((tv1).tv_sec - (tv2).tv_sec + 1e-9 * ((tv1).tv_nsec - (tv2).tv_nsec))
/** Converts a double time value (in seconds) to a timespec **/
#define DOUBLE_TO_TIMEVAL(value, tv) {	\
										(tv)->tv_sec = (int)(value); \
										(tv)->tv_nsec = ((value) - (int)(value))*1e9; \
									}

extern bool PathExists(const char * path);
extern bool DirExists(const char * path);



#endif //_COMMON_H
