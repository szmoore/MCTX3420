/**
 * @file common.h
 * @brief Common header includes
 */

#ifndef _COMMON_H
#define _COMMON_H

#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _XOPEN_SOURCE 600

/** The current API version **/
#define API_VERSION 0

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

#include "log.h"
#include "fastcgi.h"

/**Converts a timeval to a double**/
#define TIMEVAL_TO_DOUBLE(tv) ((tv).tv_sec + 1e-6 * ((tv).tv_usec))
/**Takes the tv1-tv2 between two timevals and returns the result as a double*/
#define TIMEVAL_DIFF(tv1, tv2) ((tv1).tv_sec - (tv2).tv_sec + 1e-6 * ((tv1).tv_usec - (tv2).tv_usec))

#endif //_COMMON_H
