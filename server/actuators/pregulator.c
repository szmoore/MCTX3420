#include "pregulator.h"
#include "../bbb_pin.h"

#define PREGULATOR_PWM ECAP0
#define PREGULATOR_PERIOD 500000
//16666667

/**
 * Initiliase the pressure regulator
 */
bool Pregulator_Init(const char * name, int id)
{
	return PWM_Export(PREGULATOR_PWM) && PWM_Set(PREGULATOR_PWM, false, PREGULATOR_PERIOD, 0);
}

bool Pregulator_Cleanup(int id)
{
	if (!PWM_Set(PREGULATOR_PWM, false, PREGULATOR_PERIOD, 0))
		return false;
	PWM_Unexport(PREGULATOR_PWM);
	return true;
}

bool Pregulator_Set(int id, double value)
{
	return PWM_Set(PREGULATOR_PWM, false, PREGULATOR_PERIOD, value*(PREGULATOR_PERIOD));
}

bool Pregulator_Sanity(int id, double value)
{
	return (value >= 0.0 && value <= 1.0);
}

