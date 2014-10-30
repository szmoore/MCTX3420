/**
 * @file pressure.c
 * @purpose Implementation of Pressure reading functions
 */

#include "pressure.h"
#include "../bbb_pin.h"
#include "../log.h" // For Fatal()
#include "../data.h"

#define PSI_TO_KPA 6.89475729

/** Uncalibrated values in ADC readings **/
static double high_raw[] = {368, 517, 661, 806, 950, 1093, 1232, 1378, 1518, 1660, 1800, 1942, 2000}; 
/** Calibrated values in kPa **/
static double high_cal[] = {0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 569};

static double low_raw[] = {5, 309, 390, 868, 1152, 1430, 1710, 1980, 2260};
static double low_cal[] = {0, 50, 100, 150, 200, 250, 300, 350, 400};

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
double Pressure_Calibrate(int id, int adc)
{
	switch (id)
	{
		case PRES_HIGH0:
		case PRES_HIGH1:
			return Data_Calibrate((double) adc, high_raw, high_cal, sizeof(high_raw)/sizeof(high_raw[0]));
		case PRES_LOW0:
			return Data_Calibrate((double) adc, low_raw, low_cal, sizeof(low_raw)/sizeof(low_raw[0]));
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
		*value = Pressure_Calibrate(id, adc);
		result = true;
	}
	//pthread_mutex_unlock(&mutex);
	return result;
}

/**
 * Sanity check the pressure reading
 * @param value - The pressure reading (calibrated)
 * @returns true iff the value is safe, false if it is dangerous
 */
bool Pressure_Sanity(int id, double value)
{
	return (value < 590);
}
