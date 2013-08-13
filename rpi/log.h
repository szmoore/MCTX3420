#ifndef _LOG_H
#define _LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <stdarg.h>

enum {LOGERR=0, LOGWARN=1, LOGNOTE=2, LOGINFO=3,LOGDEBUG=4};

extern void log_print(int level, char * funct, char * fmt,...);
extern void error(char * funct, char * fmt, ...);

#endif //_LOG_H

//EOF
