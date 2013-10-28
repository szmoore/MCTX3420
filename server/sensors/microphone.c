/**
 * @file microphone.c
 * @purpose Implementation of Pressure reading functions
 */

#include "microphone.h"
#include "../bbb_pin.h"
#include "../log.h" // For Fatal()

#define MIC_ADC ADC2

double adc_raw[] = {524,668,733,991,1121,1264,1300,1437,1645,1789,1932,2033,2105,2148,2284,2528,3089};
double mic_cal[] = {70,73,75,76.8,77.7,80,81.2,83.3,85.5,87.5,90.7,92.6,94.3,96.2,100,102,125};

bool Microphone_Init(const char * name, int id)
{
	assert(sizeof(adc_raw) == sizeof(mic_cal));
	return ADC_Export(MIC_ADC);
}

bool Microphone_Cleanup(int id)
{
	ADC_Unexport(MIC_ADC);
	return true;
}

bool Microphone_Read(int id, double * value)
{
	int adc = 0;
	if (!ADC_Read(MIC_ADC, &adc))
		return false;
	
	*value = Data_Calibrate((double)adc, adc_raw, mic_cal, sizeof(adc_raw)/sizeof(double));
	return true;
}

bool Microphone_Sanity(int id, double value)
{
	return true;
}
