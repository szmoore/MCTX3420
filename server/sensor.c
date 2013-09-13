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

/** Human readable names for the sensors **/
const char * g_sensor_names[NUMSENSORS] = {	
	"analog_test0", "analog_test1", 
	"analog_fail0",	"digital_test0", 
	"digital_test1", "digital_fail0"
};

/**
 * One off initialisation of *all* sensors
 */
void Sensor_Init()
{
	for (int i = 0; i < NUMSENSORS; ++i)
	{
		g_sensors[i].id = i;
		Data_Init(&(g_sensors[i].data_file));
		g_sensors[i].record_data = false;	
	}
}

/**
 * Start a Sensor recording DataPoints
 * @param s - The Sensor to start
 * @param experiment_name - Prepended to DataFile filename
 */
void Sensor_Start(Sensor * s, const char * experiment_name)
{
	// Set filename
	char filename[BUFSIZ];
	if (sprintf(filename, "%s_%d", experiment_name, s->id) >= BUFSIZ)
	{
		Fatal("Experiment name \"%s\" too long (>%d)", experiment_name, BUFSIZ);
	}

	Log(LOGDEBUG, "Sensor %d with DataFile \"%s\"", s->id, filename);
	// Open DataFile
	Data_Open(&(s->data_file), filename);

	s->record_data = true; // Don't forget this!

	// Create the thread
	pthread_create(&(s->thread), NULL, Sensor_Loop, (void*)(s));
}

/**
 * Stop a Sensor from recording DataPoints. Blocks until it has stopped.
 * @param s - The Sensor to stop
 */
void Sensor_Stop(Sensor * s)
{
	// Stop
	if (s->record_data)
	{
		s->record_data = false;
		pthread_join(s->thread, NULL); // Wait for thread to exit
		Data_Close(&(s->data_file)); // Close DataFile
		s->newest_data.time_stamp = 0;
		s->newest_data.value = 0;
	}
}

/**
 * Stop all Sensors
 */
void Sensor_StopAll()
{
	for (int i = 0; i < NUMSENSORS; ++i)
		Sensor_Stop(g_sensors+i);
}

/**
 * Start all Sensors
 */
void Sensor_StartAll(const char * experiment_name)
{
	for (int i = 0; i < NUMSENSORS; ++i)
		Sensor_Start(g_sensors+i, experiment_name);
}


/**
 * Checks the sensor data for unsafe or unexpected results 
 * @param sensor_id - The ID of the sensor
 * @param value - The value from the sensor to test
 */
void Sensor_CheckData(SensorId id, double value)
{
	switch (sensor_id)
	{
		case ANALOG_FAIL0:
		{
			if( value > ANALOG_FAIL0_SAFETY || value < ANALOG_FAIL0_MIN_SAFETY)
			{
				Log(LOGERR, "Sensor analog_fail0 is above or below its safety value of %d or %d\n", ANALOG_FAIL0_SAFETY, ANALOG_FAIL0_MIN_SAFETY);
			//new function that stops actuators?
			}
			else if( value > ANALOG_FAIL0_WARN || value < ANALOG_FAIL0_MIN_WARN)
			{
				Log(LOGWARN, "Sensor analog_test0 is above or below its warning value of %d or %d\n", ANALOG_FAIL0_WARN, ANALOG_FAIL0_MIN_WARN);	
			}
			break;
		}
		case DIGITAL_FAIL0:
		{	
			if( value != 0 && value != 1)
			{
				Log(LOGERR, "Sensor digital_fail0 is not 0 or 1\n");
			}
			break;
		}
		default:
		{
		//So it doesn't complain about the missing cases - in practice we will need all sensors to be checked as above, no need to include a default as we should only pass valid sensor_id's; unless for some reason we have a sensor we don't need to check (but then why would you pass to this function in the first place :P)
		}
	}
}


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
			d->value = (double)(rand() % 100) / 100;
			break;

		case ANALOG_TEST1:
		{
			static int count = 0;
			d->value = count++;
			break;
		}
		case ANALOG_FAIL0:
			d->value = (double)(rand() % 6) * -( rand() % 2) / ( rand() % 100 + 1);
			//Gives a value between -5 and 5
			CheckSensor(sensor_id, d->value);
			break;
		case DIGITAL_TEST0:
			d->value = t.tv_sec % 2;
			break;
		case DIGITAL_TEST1:
			d->value = (t.tv_sec+1)%2;
			break;
		case DIGITAL_FAIL0:
			if( rand() % 100 > 98)
				d->value = 2;
			d->value = rand() % 2; 
			//Gives 0 or 1 or a 2 every 1/100 times
			CheckSensor(sensor_id, d->value);
			break;
		default:
			Fatal("Unknown sensor id: %d", s->id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling

	// Perform sanity check based on Sensor's ID and the DataPoint
	Sensor_CheckData(s->id, d);

	// Update latest DataPoint if necessary
	bool result = (d->value != s->newest_data.value);
	if (result)
	{
		s->newest_data.time_stamp = d->time_stamp;
		s->newest_data.value = d->value;
	}
	return result;
}

/**
 * Record data from a single Sensor; to be run in a seperate thread
 * @param arg - Cast to Sensor* - Sensor that the thread will handle
 * @returns NULL (void* required to use the function with pthreads)
 */
void * Sensor_Loop(void * arg)
{
	Sensor * s = (Sensor*)(arg);
	Log(LOGDEBUG, "Sensor %d starts", s->id);

	// Until the sensor is stopped, record data points
	while (s->record_data)
	{
		DataPoint d;
		//Log(LOGDEBUG, "Sensor %d reads data [%f,%f]", s->id, d.time_stamp, d.value);
		if (Sensor_Read(s, &d)) // If new DataPoint is read:
		{
			//Log(LOGDEBUG, "Sensor %d saves data [%f,%f]", s->id, d.time_stamp, d.value);
			Data_Save(&(s->data_file), &d, 1); // Record it
		}
	}
	
	// Needed to keep pthreads happy

	Log(LOGDEBUG, "Sensor %d finished", s->id);
	return NULL;
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
 * Helper: Begin sensor response in a given format
 * @param context - the FCGIContext
 * @param id - ID of sensor
 * @param format - Format
 */
void Sensor_BeginResponse(FCGIContext * context, SensorId id, DataFormat format)
{
	// Begin response
	switch (format)
	{
		case JSON:
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONLong("id", id);
			FCGI_JSONKey("data");
			break;
		default:
			FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
			break;
	}
}

/**
 * Helper: End sensor response in a given format
 * @param context - the FCGIContext
 * @param id - ID of the sensor
 * @param format - Format
 */
void Sensor_EndResponse(FCGIContext * context, SensorId id, DataFormat format)
{
	// End response
	switch (format)
	{
		case JSON:
			FCGI_EndJSON();
			break;
		default:
			break;
	}
}

/**
 * Handle a request to the sensor module
 * @param context - The context to work in
 * @param params - Parameters passed
 */
void Sensor_Handler(FCGIContext *context, char * params)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	double current_time = TIMEVAL_DIFF(now, g_options.start_time);

	int id = 0;
	double start_time = 0;
	double end_time = current_time;
	const char * fmt_str;

	// key/value pairs
	FCGIValue values[] = {
		{"id", &id, FCGI_REQUIRED(FCGI_INT_T)}, 
		{"format", &fmt_str, FCGI_STRING_T}, 
		{"start_time", &start_time, FCGI_DOUBLE_T}, 
		{"end_time", &end_time, FCGI_DOUBLE_T},
	};

	// enum to avoid the use of magic numbers
	typedef enum {
		ID,
		FORMAT,
		START_TIME,
		END_TIME,
	} SensorParams;
	
	// Fill values appropriately
	if (!FCGI_ParseRequest(context, params, values, sizeof(values)/sizeof(FCGIValue)))
	{
		// Error occured; FCGI_RejectJSON already called
		return;
	}
	else if (id < 0 || id >= NUMSENSORS)
	{
		FCGI_RejectJSON(context, "Invalid sensor id specified");
		return;
	}

	// Get Sensor and format
	Sensor * s = g_sensors+id;
	DataFormat format = JSON;

	// Check if format type was specified
	if (FCGI_RECEIVED(values[FORMAT].flags))
	{
		if (strcmp(fmt_str, "json") == 0)
			format = JSON;
		else if (strcmp(fmt_str, "tsv") == 0)
			format = TSV;
		else 
		{
			FCGI_RejectJSON(context, "Unknown format type specified.");
			return;
		}
	}

	// Begin response
	Sensor_BeginResponse(context, id, format);
	
	// If a time was specified
	if (FCGI_RECEIVED(values[START_TIME].flags) || FCGI_RECEIVED(values[END_TIME].flags))
	{
		// Wrap times relative to the current time
		if (start_time < 0)
			start_time += current_time;
		if (end_time < 0)
			end_time += current_time;

		// Print points by time range
		Data_PrintByTimes(&(s->data_file), start_time, end_time, format);
	}
	else // No time was specified; just return a recent set of points
	{
		pthread_mutex_lock(&(s->data_file.mutex));
			int start_index = s->data_file.num_points-DATA_BUFSIZ;
			int end_index = s->data_file.num_points;
		pthread_mutex_unlock(&(s->data_file.mutex));

		// Bounds check
		if (start_index < 0)
			start_index = 0;
		if (end_index < 0)
			end_index = 0;

		// Print points by indexes
		Log(LOGDEBUG, "Sensor %d file \"%s\" indexes %d->%d", s->id, s->data_file.filename, start_index, end_index);
		Data_PrintByIndexes(&(s->data_file), start_index, end_index, format);
	}
	
	// Finish response
	Sensor_EndResponse(context, id, format);
	
}



