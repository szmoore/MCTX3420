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
#include "thread.h"

#endif //_COMMON_H
