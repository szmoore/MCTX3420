#ifndef _MICROPHONE_H
#define _MICROPHONE_H

#include "../common.h"
#include "../data.h"
#include <stdbool.h>

extern bool Microphone_Init(const char * name, int id);
extern bool Microphone_Cleanup(int id);
extern bool Microphone_Read(int id, double * value);
extern bool Microphone_Sanity(int id, double value);

#endif //_MICROPHONE_H


