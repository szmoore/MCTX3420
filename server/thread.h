/**
 * @file thread.h
 * @brief Declarations for thread control related functions and variables
 */

#ifndef _THREAD_H
#define _THREAD_H

#include "common.h"
#include <pthread.h>

typedef enum {QUIT, QUIT_ERROR, RUNNING} Runstate;

/** Determine if the thread should exit; to be called periodically **/
extern Runstate Thread_Runstate();
/** Tell all other threads (when they call Thread_ExitCondition) to exit. Repeated calls have no effect. **/
extern void Thread_QuitProgram(bool error);

#endif //_THREAD_H

//EOF
