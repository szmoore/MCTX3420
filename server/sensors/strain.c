#include "strain.h"

#include "../log.h"
#include "../bbb_pin.h"

#include <pthread.h>

#define STRAIN_ADC ADC0

/**
 * Convert Strain gauge id number to a GPIO pin on the Mux
 * @param id - The strain gauge id
 * @returns - GPIO pin number
 */
static int Strain_To_GPIO(StrainID id)
{
	// Could use a lookup table; that would assume strict ordering of the enum in strain.h
	//static int lookup[] = {GPIO0_30, GPIO1_28,GPIO0_31,GPIO1_16};
	//if (id < STRAIN0 || id > STRAIN3)
	//	Fatal("unknown strain id %d", id);
	//return lookup[id];

	switch (id)
	{
		case STRAIN0:
			return GPIO0_30;
		case STRAIN1:
			return GPIO1_28;
		case STRAIN2:
			return GPIO0_31;
		case STRAIN3:
			return GPIO1_16;
		default:
			Fatal("Unknown StrainID %d", id);
			return -1; // Should never happen
	}
}

/**
 * Convert ADC Strain reading to a physically meaningful value
 * @param reading - The ADC reading
 * @returns Something more useful
 */
static double Strain_Calibrated(int reading)
{
	return (double)(reading);
}

/**
 * Initialise a Strain gauge
 * @param id - The strain gauge to initialise
 * @param name - Name of the strain gauge; ignored
 * @returns true if initialisation was successful
 */
bool Strain_Init(const char * name, int id)
{
	int gpio_num = Strain_To_GPIO(id);
	GPIO_Export(gpio_num);
	if (!GPIO_Set(gpio_num, false))
		Fatal("Couldn't set GPIO%d for strain sensor %d to LOW", gpio_num, id);

	static bool init_adc = false;
	if (!init_adc)
	{
		init_adc = true;
		ADC_Export(STRAIN_ADC);
	}
	return true;
}

/**
 * Read from a Strain gauge
 * @param id - The strain gauge to read
 * @param val - Will store the read value if successful
 * @returns true on successful read
 */
bool Strain_Read(int id, double * value)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // CRITICAL SECTION - Only one Strain gauge can be read at a time
	pthread_mutex_lock(&mutex);

	int gpio_num = Strain_To_GPIO(id);
	
	// Set the multiplexer
	if (!GPIO_Set(gpio_num, true))
		Fatal("Couldn't set GPIO%d for strain sensor %d to HIGH (before reading)", gpio_num,id);

	// Read the ADC
	int tmp = 0;
	bool result = ADC_Read(STRAIN_ADC, &tmp); // If this fails, it's not fatal
	
	//TODO: Callibrate?
	*value = Strain_Calibrated(tmp);

	// Unset the multiplexer
	if (!GPIO_Set(gpio_num, false))
		Fatal("Couldn't set GPIO%d for strain sensor %d to LOW (after reading)", gpio_num, id);

	pthread_mutex_unlock(&mutex);

	return result;
}
