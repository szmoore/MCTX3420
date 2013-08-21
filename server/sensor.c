/**
 * @file sensor.c
 * @purpose Implementation of sensor thread
 * TODO: Finalise implementation
 */


#include "common.h"
#include "sensor.h"
#include <math.h>

/**
 * Read a data value from a sensor; block until value is read
 * @param sensor_id - The ID of the sensor
 * @returns The current value of the sensor
 */
DataPoint GetData(int sensor_id)
{
	// switch based on the sensor_id at the moment for testing;
	// might be able to just directly access ADC from sensor_id?
	//TODO: Implement for real sensors

	DataPoint d;
	//TODO: Deal with time stamps properly
	static int count = 0;
	d.time = count++;
	switch (sensor_id)
	{
		case SENSOR_TEST0:
			d.value = count;
			break;
		case SENSOR_TEST1:
			d.value = (float)(rand() % 100) / 100;
			break;
		default:
			Fatal("Unknown sensor id: %d", sensor_id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling
	return d;
}


/**
 * Destroy a sensor
 * @param s - Sensor to destroy
 */
void Destroy(Sensor * s)
{
	// Maybe move the binary file into long term file storage?
	fclose(s->file);
}



/**
 * Initialise a sensor
 * @param s - Sensor to initialise
 */
void Sensor_Init(Sensor * s, int id)
{
	s->write_index = 0;
	s->read_offset = 0;
	s->id = id;

	#define FILENAMESIZE BUFSIZ
	char filename[FILENAMESIZE];
	//if (s->id >= pow(10, FILENAMESIZE))
	if (false)
	{
		Fatal("Too many sensors! FILENAMESIZE is %d; increase it and recompile.", FILENAMESIZE);
	}

	pthread_mutex_init(&(s->mutex), NULL);
		
	sprintf(filename, "%d", s->id);
	unlink(filename); //TODO: Move old files somewhere

	s->file = fopen(filename, "a+b"); // open binary file
	Log(LOGDEBUG, "Initialised sensor %d; binary file is \"%s\"", id, filename);
}


/**
 * Run the main sensor polling loop
 * @param arg - Cast to Sensor* - Sensor that the thread will handle
 * @returns NULL (void* required to use the function with pthreads)
 */
void * Sensor_Main(void * arg)
{
	Sensor * s = (Sensor*)(arg);

	while (true) //TODO: Exit condition
	{
		// The sensor will write data to a buffer until it is full
		// Then it will open a file and dump the buffer to the end of it.
		// Rinse and repeat

		// The reason I've added the buffer is because locks are expensive
		// But maybe it's better to just write data straight to the file
		// I'd like to do some tests by changing SENSOR_DATABUFSIZ

		while (s->write_index < SENSOR_DATABUFSIZ)
		{
			s->buffer[s->write_index] = GetData(s->id);
			s->write_index += 1;
		}

		//Log(LOGDEBUG, "Filled buffer");

		// CRITICAL SECTION (no threads should be able to read/write the file at the same time)
		pthread_mutex_lock(&(s->mutex));
			fseek(s->file, 0, SEEK_END);
			int amount_written = fwrite(s->buffer, sizeof(DataPoint), SENSOR_DATABUFSIZ, s->file);
			if (amount_written != SENSOR_DATABUFSIZ)
			{
				Fatal("Wrote %d data points and expected to write %d to \"%s\" - %s", amount_written, SENSOR_DATABUFSIZ, strerror(errno));
			}
			//Log(LOGDEBUG, "Wrote %d data points for sensor %d", amount_written, s->id);
		pthread_mutex_unlock(&(s->mutex));
		// End of critical section

		s->write_index = 0; // reset position in buffer
		
	}
	return NULL;
}

/**
 * Fill buffer with most recent sensor data
 * @param s - Sensor to use
 * @param buffer - Buffer to fill
 * @param bufsiz - Size of buffer to fill
 * @returns The number of DataPoints actually read
 */
int Sensor_Query(Sensor * s, DataPoint * buffer, int bufsiz)
{
	int amount_read = 0;
	//CRITICAL SECTION (Don't access file while sensor thread is writing to it!)
	pthread_mutex_lock(&(s->mutex));
		
		fseek(s->file, -bufsiz*sizeof(DataPoint), SEEK_END);
		amount_read = fread(buffer, sizeof(DataPoint), bufsiz, s->file);
		//Log(LOGDEBUG, "Read %d data points", amount_read);		
	pthread_mutex_unlock(&(s->mutex));
	return amount_read;
}

/**
 * Handle a request to the sensor module
 * @param context - The context to work in
 * @param params - Parameters passed
 */
void Sensor_Handler(FCGIContext *context, char * params)
{
	DataPoint buffer[SENSOR_QUERYBUFSIZ];
	StatusCodes status = STATUS_OK;
	const char * key; const char * value;

	int sensor_id = SENSOR_NONE;

	while ((params = FCGI_KeyPair(params, &key, &value)) != NULL)
	{
		Log(LOGDEBUG, "Got key=%s and value=%s", key, value);
		if (strcmp(key, "id") == 0)
		{
			char *end;
			if (sensor_id != SENSOR_NONE)
			{
				Log(LOGERR, "Only one sensor id should be specified");
				status = STATUS_ERROR;
				break;
			}
			if (*value == '\0')
			{
				Log(LOGERR, "No id specified.");
				status = STATUS_ERROR;
				break;
			}
			//TODO: Use human readable sensor identifier string for API?
			sensor_id = strtol(value, &end, 10);
			if (*end != '\0')
			{
				Log(LOGERR, "Sensor id not an integer; %s", value);
				status = STATUS_ERROR;
				break;
			}
		}
		else
		{
			Log(LOGERR, "Unknown key \"%s\" (value = %s)", key, value);
			status = STATUS_ERROR;
			break;
		}		
	}

	if (sensor_id == SENSOR_NONE)
	{
		Log(LOGERR, "No sensor id specified");
		status = STATUS_ERROR;
	}
	else if (sensor_id >= NUMSENSORS || sensor_id < 0)
	{
		Log(LOGERR, "Invalid sensor id %d", sensor_id);
		status = STATUS_ERROR;
	}

	if (status == STATUS_ERROR)
	{
		FCGI_RejectJSON(context);
	}
	else
	{
		FCGI_BeginJSON(context, status);	
		FCGI_JSONPair(key, value); // should spit back sensor ID
		//Log(LOGDEBUG, "Call Sensor_Query...");
		int amount_read = Sensor_Query(&(g_sensors[sensor_id]), buffer, SENSOR_QUERYBUFSIZ);
		//Log(LOGDEBUG, "Read %d DataPoints", amount_read);
		//Log(LOGDEBUG, "Produce JSON response");
		FCGI_JSONKey("data");
		FCGI_JSONValue("[");
		for (int i = 0; i < amount_read; ++i)
		{
			FCGI_JSONValue("[%f, %f]", buffer[i].time, buffer[i].value);
			if (i+1 < amount_read)
				FCGI_JSONValue(",");
		}
		FCGI_JSONValue("]");
		//Log(LOGDEBUG, "Done producing JSON response");
		FCGI_EndJSON();	
	}
}
