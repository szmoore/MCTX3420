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
	"digital_test0", "digital_test1"
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
	if (sprintf(filename, "%s_s%d", experiment_name, s->id) >= BUFSIZ)
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
		case DIGITAL_TEST0:
			d->value = t.tv_sec % 2;
			break;
		case DIGITAL_TEST1:
			d->value = (t.tv_sec+1)%2;
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
 * Checks the sensor data for unsafe or unexpected results 
 * @param sensor_id - The ID of the sensor
 * @param d - DataPoint to check
 */
void Sensor_CheckData(SensorId id, DataPoint * d)
{
	//TODO: Implement
	/*
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
 * @param format - Format
 * @param id - ID of sensor
 */
void Sensor_BeginResponse(FCGIContext * context, SensorId id, DataFormat format)
{
	// Begin response
	switch (format)
	{
		case JSON:
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONLong("id", id);
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
	char * fmt_str;

	// key/value pairs
	FCGIValue values[] = {
		{"id", &id, FCGI_REQUIRED(FCGI_LONG_T)}, 
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


	// Error checking on sensor id
	if (id < 0 || id >= NUMSENSORS)
	{
		FCGI_RejectJSON(context, "Invalid sensor id");
		return;
	}
	Sensor * s = g_sensors+id;
	
	DataFormat format = Data_GetFormat(&(values[FORMAT]));

	// Begin response
	Sensor_BeginResponse(context, id, format);

	// Print Data
	Data_Handler(&(s->data_file), &(values[START_TIME]), &(values[END_TIME]), format, current_time);
	
	// Finish response
	Sensor_EndResponse(context, id, format);
	
}


