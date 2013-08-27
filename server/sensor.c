/**
 * @file sensor.c
 * @purpose Implementation of sensor thread
 * TODO: Finalise implementation
 */


#include "common.h"
#include "sensor.h"
#include <math.h>

/** Array of sensors, initialised by Sensor_Init **/
static Sensor g_sensors[NUMSENSORS]; //global to this file

/**
 * Read a data value from a sensor; block until value is read
 * @param sensor_id - The ID of the sensor
 * @param d - DataPoint to set
 * @returns NULL on error, otherwise d
 */
DataPoint * GetData(int sensor_id, DataPoint * d)
{
	// switch based on the sensor_id at the moment for testing;
	// might be able to just directly access ADC from sensor_id?
	//TODO: Implement for real sensors

	
	//TODO: We should ensure the time is *never* allowed to change on the server if we use gettimeofday
	//		Another way people might think of getting the time is to count CPU cycles with clock()
	//		But this will not work because a) CPU clock speed may change on some devices (RPi?) and b) It counts cycles used by all threads
	gettimeofday(&(d->time_stamp), NULL);
	
	switch (sensor_id)
	{
		case SENSOR_TEST0:
		{
			static int count = 0;
			d->value = count++;
			break;
		}
		case SENSOR_TEST1:
			d->value = (float)(rand() % 100) / 100;
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
void Init(Sensor * s, int id)
{
	s->write_index = 0;
	s->read_offset = 0;
	s->id = id;

	#define FILENAMESIZE 3
	char filename[FILENAMESIZE];
	if (s->id >= pow(10, FILENAMESIZE))
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

	while (Thread_Runstate() == RUNNING) //TODO: Exit condition
	{
		// The sensor will write data to a buffer until it is full
		// Then it will open a file and dump the buffer to the end of it.
		// Rinse and repeat

		// The reason I've added the buffer is because locks are expensive
		// But maybe it's better to just write data straight to the file
		// I'd like to do some tests by changing SENSOR_DATABUFSIZ

		while (s->write_index < SENSOR_DATABUFSIZ)
		{
			DataPoint * d = &(s->buffer[s->write_index]);
			if (GetData(s->id, d) == NULL)
			{
				Fatal("Error collecting data");
			}
			s->write_index += 1;
		}

		//Log(LOGDEBUG, "Filled buffer");

		// CRITICAL SECTION (no threads should be able to read/write the file at the same time)
		pthread_mutex_lock(&(s->mutex));
			//TODO: Valgrind complains about this fseek: "Syscall param write(buf) points to uninitialised byte(s)"
			//		Not sure why, but we should find out and fix it.
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
	Log(LOGDEBUG, "Thread for sensor %d exits", s->id);
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
 * Get a Sensor given an ID string
 * @param id_str ID string
 * @returns Sensor* identified by the string; NULL on error
 */
Sensor * Sensor_Identify(const char * id_str)
{
	char * end;
	// Parse string as integer
	int id = strtol(id_str, &end, 10);
	if (*end != '\0')
	{
		return NULL;
	}
	// Bounds check
	if (id < 0 || id > NUMSENSORS)
		return NULL;

	return g_sensors+id;
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

	Sensor * sensor = NULL;

	while ((params = FCGI_KeyPair(params, &key, &value)) != NULL)
	{
		Log(LOGDEBUG, "Got key=%s and value=%s", key, value);
		if (strcmp(key, "id") == 0)
		{
			if (sensor != NULL)
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

			sensor = Sensor_Identify(value);
			if (sensor == NULL)
			{
				Log(LOGERR, "Invalid sensor id: %s", value);
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

	if (status != STATUS_ERROR && sensor == NULL)
	{
		Log(LOGERR, "No valid sensor id given");
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
		int amount_read = Sensor_Query(sensor, buffer, SENSOR_QUERYBUFSIZ);
		//Log(LOGDEBUG, "Read %d DataPoints", amount_read);
		//Log(LOGDEBUG, "Produce JSON response");
		FCGI_JSONKey("data");
		FCGI_JSONValue("[");
		for (int i = 0; i < amount_read; ++i)
		{
			//TODO: Consider; is it better to give both tv_sec and tv_usec to the client seperately, instead of combining here?
			//NOTE: Must always use doubles; floats get rounded!
			double time = buffer[i].time_stamp.tv_sec + 1e-6*(buffer[i].time_stamp.tv_usec);
			FCGI_JSONValue("[%f, %f]", time, buffer[i].value);
			if (i+1 < amount_read)
				FCGI_JSONValue(",");
		}
		FCGI_JSONValue("]");
		//Log(LOGDEBUG, "Done producing JSON response");
		FCGI_EndJSON();	
	}
}

/**
 * Setup Sensors, start Sensor polling thread(s)
 */
void Sensor_Spawn()
{
	// start sensor threads
	for (int i = 0; i < NUMSENSORS; ++i)
	{
		Init(g_sensors+i, i);
		pthread_create(&(g_sensors[i].thread), NULL, Sensor_Main, (void*)(g_sensors+i));
	}
}

/**
 * Quit Sensor loops
 */
void Sensor_Join()
{
	if (!Thread_Runstate())
	{
		Fatal("This function should not be called before Thread_QuitProgram");
	}
	for (int i = 0; i < NUMSENSORS; ++i)
	{
		pthread_join(g_sensors[i].thread, NULL);
		Destroy(g_sensors+i);
	}
}
