#ifndef _PREGULATOR
#define _PREGULATOR

#include "../common.h"

#define PREGULATOR_PWM ECAP0
#define PREGULATOR_PERIOD 500000
//16666667

extern bool Pregulator_Init(const char * name, int id);
extern bool Pregulator_Cleanup(int id);
extern bool Pregulator_Set(int id, double value);
extern bool Pregulator_Sanity(int id, double value);


#endif //_PREGULATOR
