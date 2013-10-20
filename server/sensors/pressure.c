/**
 * @file pressure.c
 * @purpose Implementation of Pressure reading functions
 */

#include "pressure.h"
#include "../bbb_pin.h"
#include "../log.h" // For Fatal()

#define PSI_TO_KPA 6.89475729

/**
 * Get the ADC number of a Pressure sensor
 * @param id - Id of the sensor
 * @returns ADC as defined in bbb_pin_defines.h
 */
static int Pressure_GetADC(int id)
{
	switch (id)
	{
		case PRES_HIGH0:
			return ADC1;
		case PRES_HIGH1:
			return ADC3;
		case PRES_LOW0:
			return ADC5;
		default:
			Fatal("Unknown Pressure id %d", id);
			return -1; // Should never happen
	}
}

/**
 * Convert an ADC voltage into a Pressure reading
 * @param id - Sensor ID
 * @param adc - ADC reading
 * @returns Pressure in kPa
 */
double Pressure_Callibrate(int id, int adc)
{
	//double voltage = ADC_TO_VOLTS(adc); // convert reading to voltage

	switch (id)
	{
		case PRES_HIGH0:
		case PRES_HIGH1:
		{
			static const double Vs = 5e3; // In mVs
			static const double Pmin = 0.0 * PSI_TO_KPA;
			static const double Pmax = 150.0 * PSI_TO_KPA;
			double Vout = ADC_TO_MVOLTS(adc);
			return ((Vout - 0.1*Vs)/(0.8*Vs))*(Pmax - Pmin) + Pmin;
		}	
		case PRES_LOW0:
			return (200.0 * (adc / ADC_RAW_MAX));
		default:
			Fatal("Unknown Pressure id %d", id);
			return -1; // Should never happen
	}
	
}

/**
 * Initialise a Pressure sensor
 * @param name - Ignored
 * @param id - The id of the Pressure sensor
 * @returns true on success, false on error
 */
bool Pressure_Init(const char * name, int id)
{
	return ADC_Export(Pressure_GetADC(id));
}

/**
 * Cleanup a Pressure Sensor
 * @param id - The id of the sensor to cleanup
 * @returns true on success, false on failure
 */
bool Pressure_Cleanup(int id)
{
	ADC_Unexport(Pressure_GetADC(id));
	return true;
}

/**
 * Read a Pressure Sensor
 * @param id - id of the sensor to read
 * @param value - Where the value will be stored on a successful read
 * @returns true on success, false on failure
 */
bool Pressure_Read(int id, double * value)
{
	//static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	//pthread_mutex_lock(&mutex);
	bool result = false;
	int adc = 0;
	if (ADC_Read(Pressure_GetADC(id), &adc))
	{
		*value = Pressure_Callibrate(id, adc);
		result = true;
	}
	//pthread_mutex_unlock(&mutex);
	return result;
}
