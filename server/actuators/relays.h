#ifndef _RELAY_H
#define _RELAY_H

#include "../common.h"

typedef enum
{
	RELAY_CANSELECT,
	RELAY_CANENABLE,
	RELAY_MAIN
} RelayID;

extern bool Relay_Init(const char * name, int id);
extern bool Relay_Set(int id, double value);
extern bool Relay_Sanity(int id, double value);
extern bool Relay_Cleanup(int id);

#endif //_RELAY_H
