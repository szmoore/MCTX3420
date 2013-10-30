#include "pregulator.h"
#include "../bbb_pin.h"


#include "../data.h"
#define PREGULATOR_PWM ECAP0
#define PREGULATOR_PERIOD 500000
//16666667

/** PWM duty cycles raw **/
static double pwm_raw[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
/** Calibrated pressure values match with pwm_raw **/
static double preg_cal[] = {96, 190, 285, 380, 475, 569};

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
	double anti_calibrated = Data_Calibrate(value, preg_cal, pwm_raw, sizeof(pwm_raw)/sizeof(double));
	Log(LOGDEBUG, "Pregulator value %f -> PWM duty cycle %f", value, anti_calibrated);
	if (anti_calibrated < 0)
		anti_calibrated = 0;
	if (anti_calibrated > 1)
		anti_calibrated = 1;
	return PWM_Set(PREGULATOR_PWM, false, PREGULATOR_PERIOD, anti_calibrated*(PREGULATOR_PERIOD));
}

bool Pregulator_Sanity(int id, double value)
{
	return (value >= 0 && value < 570);
}

