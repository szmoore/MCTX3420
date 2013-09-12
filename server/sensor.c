/**
 * @file sensor.c
 * @brief Implementation of sensor thread
 * TODO: Finalise implementation
 */

#include "common.h"
#include "sensor.h"
#include "options.h"
#include <math.h>

/** Array of sensors, initialised by Sensor_Init **/
static Sensor g_sensors[NUMSENSORS]; //global to this file
const char * g_sensor_names[NUMSENSORS] = {	
	"analog_test0", "analog_test1", 
	"digital_test0", "digital_test1"
};

/**
 * Read a data value from a sensor; block until value is read
 * @param sensor_id - The ID of the sensor
 * @param d - DataPoint to set
 * @returns NULL for digital sensors when data is unchanged, otherwise d
 */
DataPoint * GetData(SensorId sensor_id, DataPoint * d)
{
	// switch based on the sensor_id at the moment for testing;
	// might be able to just directly access ADC from sensor_id?
	//TODO: Implement for real sensors

	
	//TODO: We should ensure the time is *never* allowed to change on the server if we use gettimeofday
	//		Another way people might think of getting the time is to count CPU cycles with clock()
	//		But this will not work because a) CPU clock speed may change on some devices (RPi?) and b) It counts cycles used by all threads
	
	struct timeval t;
	gettimeofday(&t, NULL);
	d->time_stamp = TIMEVAL_DIFF(t, g_options.start_time);

	// Make time relative
	//d->time_stamp.tv_sec -= g_options.start_time.tv_sec;
	//d->time_stamp.tv_usec -= g_options.start_time.tv_usec;
	
	switch (sensor_id)
	{
		case ANALOG_TEST0:
		{
			//CheckSensor( sensor_id, *sensor value*); 
		
			static int count = 0;
			d->value = count++;
			break;
		}
		case ANALOG_TEST1:
			d->value = (double)(rand() % 100) / 100;
			break;
	
		//TODO: For digital sensors, consider only updating when sensor is actually changed
		case DIGITAL_TEST0:
			d->value = t.tv_sec % 2;
			break;
		case DIGITAL_TEST1:
			d->value = (t.tv_sec+1)%2;
			break;
		default:
			Fatal("Unknown sensor id: %d", sensor_id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling

	return d;
}

/**
 * Checks the sensor data for unsafe or unexpected results 
 * @param sensor_id - The ID of the sensor
 *
*
void CheckSensor( SensorId sensor_id)
{
	switch (sensor_id)
	{
		case ANALOG_TEST0:
		{
			if( *sensor value* > ANALOG_TEST0_SAFETY)
			{
				LogEx(LOGERR, GetData, Sensor analog_test0 is above the safe value);
			//new log function that stops actuators?
			}
			//Also include a warning level?
			else if( *sensor value* > ANALOG_TEST0_WARN)
			{
				LogEx(LOGWARN, GetData, Sensor analog_test0);	
			}
		}
	}
		
	
*/	


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
	s->id = id;
	s->points_written = 0;
	s->points_read = 0;

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
			s->points_written += amount_written;
			//Log(LOGDEBUG, "Wrote %d data points for sensor %d", amount_written, s->id);
		pthread_mutex_unlock(&(s->mutex));
		// End of critical section

		s->write_index = 0; // reset position in buffer
		
	}
	Log(LOGDEBUG, "Thread for sensor %d exits", s->id);
	return NULL; 
}

/**
 * Get position in a binary sensor file with a timestamp using a binary search
 * @param s - Sensor to use
 * @param time_stamp - Timestamp
 * @param count - If not NULL, used to provide number of searches required
 * @param found - If not NULL, set to the closest DataPoint
 * @returns Integer giving the *closest* index in the file
 * TODO: Refactor or replace?
 */
int FindTime(Sensor * s, double time_stamp, int * count, DataPoint * found)
{
	DataPoint d;

	int lower = 0;
	int upper = s->points_written - 1;
	int index = 0;
	if (count != NULL)
		*count = 0;	

	while (upper - lower > 1)
	{
		index = lower + ((upper - lower)/2);

		// Basically anything with fseek is critical; if we don't make it critical the sensor thread may alter data at a random point in the file!
		// CRITICAL SECTION (May need to rethink how this is done, but I can't see how to do it without fseek :S)
		// Regarding the suggestion that we have 2 file pointers; one for reading and one for writing:
		// That seems like it will work... but we will have to be very careful and test it first
		pthread_mutex_lock(&s->mutex);
			fseek(s->file, index*sizeof(DataPoint), SEEK_SET);
			int amount_read = fread(&d, sizeof(DataPoint), 1, s->file);
		pthread_mutex_unlock(&s->mutex);
		
		if (amount_read != 1)
		{
			Fatal("Couldn't read single data point from sensor %d", s->id);
		}

		if (d.time_stamp > time_stamp)
		{
			upper = index;
		}
		else if (d.time_stamp < time_stamp)
		{
			lower = index;
		}
		if (count != NULL)
			*count += 1;
	}

	if (found != NULL)
		*found = d;

	return index;
	
}

/**
 * Print sensor data between two indexes in the file, using a given format
 * @param s - Sensor to use
 * @param start - Start index
 * @param end - End index
 * @param output_type - JSON, CSV or TSV output format
 */
void PrintData(Sensor * s, int start, int end, OutputType output_type)
{
	DataPoint buffer[SENSOR_QUERYBUFSIZ];
	int index = start;

	if (output_type == JSON)
	{
		FCGI_JSONValue("[");
	}


	while (index < end)
	{
		int to_read = end - index;
		if (to_read > SENSOR_QUERYBUFSIZ)
		{
			to_read = SENSOR_QUERYBUFSIZ;
		}

		int amount_read = 0;
		// CRITICAL SECTION
		pthread_mutex_lock(&(s->mutex));

			fseek(s->file, index*sizeof(DataPoint), SEEK_SET);
			amount_read = fread(buffer, sizeof(DataPoint), to_read, s->file);

		pthread_mutex_unlock(&(s->mutex));
		// End critical section

		if (amount_read != to_read)
		{
			Fatal("Failed to read %d DataPoints from sensor %d; read %d instead", to_read, s->id, amount_read);
		}

		// Print the data
		for (int i = 0; i < amount_read; ++i)
		{
			//TODO: Reformat?
			switch (output_type)
			{
				case JSON:
					FCGI_JSONValue("[%f, %f]", buffer[i].time_stamp, buffer[i].value);
					if (i+1 < amount_read)
						FCGI_JSONValue(",");
					break;
				case CSV:
					FCGI_PrintRaw("%f,%f\n", buffer[i].time_stamp, buffer[i].value);
					break;
				case TSV:
				default:
					FCGI_PrintRaw("%f\t%f\n", buffer[i].time_stamp, buffer[i].value);
					break;
			}
		}
		index += amount_read;
	}

	if (output_type == JSON)
	{
		FCGI_JSONValue("]");
	}
}

/**
 * Fill buffer with most recent sensor data
 * TODO: This may be obselete; remove?
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
	if (id < 0 || id >= NUMSENSORS)
		return NULL;


	Log(LOGDEBUG, "Sensor \"%s\" identified", g_sensor_names[id]);
	return g_sensors+id;
}

/**
 * Handle a request to the sensor module
 * @param context - The context to work in
 * @param params - Parameters passed
 * TODO: Seriously need to write more helper functions and decrease the size of this function!
 */
void Sensor_Handler(FCGIContext *context, char * params)
{
	StatusCodes status = STATUS_OK;

	OutputType output_type = JSON;
	


	const char * key; const char * value;

	Sensor * sensor = NULL;

	struct timeval now;
	gettimeofday(&now, NULL);

	double start_time = -1;
	double end_time = -1;
	double current_time = TIMEVAL_DIFF(now, g_options.start_time)
	bool seek_time = false;
	bool points_specified = false;
	int query_size = SENSOR_QUERYBUFSIZ;
	int start_index = -1;
	int end_index = -1;

	/* //Possible use case?
	FCGIValue values[5] = {
		{"id", &id, FCGI_REQUIRED(FCGI_INT_T)},
		{"format", &format, FCGI_STRING_T},
		{"points", &points, FCGI_STRING_T},
		{"start_time", &start_time, FCGI_DOUBLE_T},
		{"end_time", &end_time, FCGI_DOUBLE_T}
	};
	if (!FCGI_ParseRequest(context, params, values, 5))
		return;*/

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
		else if (strcmp(key, "format") == 0)
		{
			if (strcmp(value, "json") == 0)
				output_type = JSON;
			else if (strcmp(value, "csv") == 0)
				output_type = CSV;
			else if (strcmp(value, "tsv") == 0)
				output_type = TSV;			
		}
		else if (strcmp(key, "points") == 0)
		{
			points_specified = true;
			if (strcmp(value, "all") == 0)
			{
				query_size = sensor->points_written;
			}
			else
			{
				char * end;
				query_size = strtol(value, &end, 10);
				if (*end != '\0')
				{
					Log(LOGERR, "Require \"all\" or an integer value: %s = %s", key, value);
					status = STATUS_ERROR;
					break;
				}
			}
			
		}
		else if (strcmp(key, "start_time") == 0)
		{
			seek_time = true;
			char * end;
			start_time = strtod(value, &end);
			if (*end != '\0')
			{
				Log(LOGERR, "Require a double: %s = %s", key, value);
				status = STATUS_ERROR;
				break;
			}			

			// Treat negative values as being relative to the current time
			if (start_time < 0)
			{
				start_time = current_time + start_time;
			}
			start_time = floor(start_time);
		}
		else if (strcmp(key, "end_time") == 0)
		{
			seek_time = true;
			char * end;
			end_time = strtod(value, &end);
			if (*end != '\0')
			{
				Log(LOGERR, "Require a double: %s = %s", key, value);
				status = STATUS_ERROR;
				break;
			}	

			// Treat negative values as being relative to the current time
			if (end_time < 0)
			{
				end_time = current_time + end_time;
			}		
			end_time = ceil(end_time);
		}
		// For backward compatability:
		else if (strcmp(key, "dump") == 0)
		{
			output_type = TSV;
			query_size = sensor->points_written+1;
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
		FCGI_RejectJSON(context, "Invalid input parameters");
		return;
	}


	if (seek_time)
	{
		if (end_time < 0 && !points_specified)
			end_index = sensor->points_written;
		else
		{
			int count = 0; DataPoint d;
			end_index = FindTime(sensor, end_time, &count, &d);
			Log(LOGDEBUG, "FindTime - Looked for %f; found [%f,%f] after %d iterations; sensor %d, position %d", end_time, d.time_stamp, d.value, count, sensor->id, end_index);
		}
		if (start_time < 0)
			start_time = 0;
		else
		{
			int count = 0; DataPoint d;
			start_index = FindTime(sensor, start_time, &count, &d);
			Log(LOGDEBUG, "FindTime - Looked for %f; found [%f,%f] after %d iterations; sensor %d, position %d", start_time, d.time_stamp, d.value, count, sensor->id, start_index);
		}

		if (points_specified)
			end_index = start_index + query_size;
	}
	else
	{
		start_index = sensor->points_written - query_size;
		
		end_index = sensor->points_written;
	}
	
	if (start_index < 0)
	{
		Log(LOGNOTE, "start_index = %d => Clamped to 0", start_index);
		start_index = 0;
	}
	if (end_index > sensor->points_written)
	{
		Log(LOGNOTE, "end_index = %d => Clamped to %d", end_index, sensor->points_written);
		end_index = sensor->points_written;
	}
	
	switch (output_type)
	{
		case JSON:
			FCGI_BeginJSON(context, status);
			FCGI_JSONLong("id", sensor->id);
			FCGI_JSONKey("data");
			PrintData(sensor, start_index, end_index, output_type);
			FCGI_EndJSON();
			break;
		default:
			FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
			PrintData(sensor, start_index, end_index, output_type);
			//Force download with content-disposition
			// Sam: This is cool, but I don't think we should do it
			//  - letting the user view it in the browser and then save with their own filename is more flexible
			//"Content-disposition: attachment;filename=%d.csv\r\n\r\n", sensor->id);
			break;
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
