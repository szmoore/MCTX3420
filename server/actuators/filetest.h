#ifndef _FILETEST_H
#define _FILETEST_H

#include "../common.h"

extern bool Filetest_Init(const char * name, int id);
extern bool Filetest_Set(int id, double value);
extern bool Filetest_Cleanup(int id);
extern bool Filetest_Sanity(int id, double value);

#endif //_FILETEST_H


