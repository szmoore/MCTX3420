/**
 * @file common.h
 * @purpose Common header includes
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

extern void Handler_Sensors(FCGIContext *data, char *params);

#endif //_COMMON_H

