/**
 * @file query.c
 * @purpose Temporary file to run a test thread that will query a sensors thread
 * Code will probably be combined with Jeremy's FastCGI API
 */



#include "query.h"

#include "sensor.h"
#include "log.h"

static DataPoint buffer[QUERY_BUFSIZ];

/**
 * Query sensor with id
 * @param id - The index of the sensor in g_sensors
 */
void QuerySensor(int id) //TODO: This code will form the SensorHandler FastCGI function (I think?)
{
	Sensor * s = g_sensors+id;

	int amount_read = 0;
	//CRITICAL SECTION (Don't access file while sensor thread is writing to it!)
	pthread_mutex_lock(&(s->mutex));

		fseek(s->file, -QUERY_BUFSIZ*sizeof(DataPoint), SEEK_END);
		amount_read = fread(&buffer, sizeof(DataPoint), QUERY_BUFSIZ, s->file);
		Log(LOGDEBUG, "Read %d data points", amount_read);
		
	pthread_mutex_unlock(&(s->mutex));
	//End critical section

	// So... we have a buffer
	// I guess we'll want to JSON it or something?
	// Just print it out for now
	for (int i = 0; i < amount_read; ++i)
	{
		printf("%f\t%f\n", buffer[i].time, buffer[i].value);
	}

	// Will want to handle case where there actually wasn't anything new to respond with
	// (In case we have a sensor that is slower than the rate of jQuery requests)
	if (amount_read == 0)
	{
		Log(LOGWARN, "No data points read from sensor%s file");
		printf("# No data\n");
	}
}

/**
 * Test function to simulate responding to HTTP requests
 * @param args - IGNORED (void* required to pass function to pthread_create)
 * @returns NULL (void* required to pass function to pthread_create)
 */
void * Query_Main(void * args)
{
	while (true) //TODO: Exit condition
	{
		
		for (int i = 0; i < NUMSENSORS; ++i)
		{
			printf("# Sensor %d\n", i);
			QuerySensor(i);
			printf("\n");	
		}
		usleep(REQUEST_RATE);
	}
}
