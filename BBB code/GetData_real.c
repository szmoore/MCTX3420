#include "gpio.h"

/**
 * Read a DataPoint from a Sensor; block until value is read
 * @param id - The ID of the sensor
 * @param d - DataPoint to set
 * @returns True if the DataPoint was different from the most recently recorded.
 */
bool Sensor_Read(Sensor * s, DataPoint * d)
{
	
	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);
	d->time_stamp = TIMEVAL_DIFF(t, g_options.start_time);

	// Read value based on Sensor Id
	switch (s->id)
	{
		case ANALOG_TEST0:
			d->value = ADCRead(0);	//ADC #0 on the Beaglebone
			break;
		case ANALOG_TEST1:
		{
			d->value = ADCRead(1);
			break;
		}
		case ANALOG_FAIL0:
			d->value = (double)(rand() % 6) * -( rand() % 2) / ( rand() % 100 + 1);
			//Gives a value between -5 and 5
			break;
		case DIGITAL_TEST0:
			d->value = pinRead(0);	//replace 0 with correct pin number
			break;
		case DIGITAL_TEST1:
			d->value = pinRead(1);	//replace 1 with correct pin number
			break;
		case DIGITAL_FAIL0:
			if( rand() % 100 > 98)
				d->value = 2;
			d->value = rand() % 2; 
			//Gives 0 or 1 or a 2 every 1/100 times
			break;
		default:
			Fatal("Unknown sensor id: %d", s->id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling

	// Perform sanity check based on Sensor's ID and the DataPoint
	Sensor_CheckData(s->id, d->value);

	// Update latest DataPoint if necessary
	bool result = (d->value != s->newest_data.value);
	if (result)
	{
		s->newest_data.time_stamp = d->time_stamp;
		s->newest_data.value = d->value;
	}
	return result;
}