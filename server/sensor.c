/**
 * @file sensor.c
 * @brief Implementation of sensor thread
 * TODO: Finalise implementation
 */

#include "common.h"
#include "sensor.h"
#include "options.h"
#include "bbb_pin.h"
#include <math.h>

/** Array of sensors, initialised by Sensor_Init **/
static Sensor g_sensors[SENSORS_MAX];
/** The number of sensors **/
int g_num_sensors = 0;



/** 
 * Add and initialise a Sensor
 * @param name - Human readable name of the sensor
 * @param user_id - User identifier
 * @param read - Function to call whenever the sensor should be read
 * @param init - Function to call to initialise the sensor (may be NULL)
 * @param cleanup - Function to call whenever to deinitialise the sensor (may be NULL)
 * @param sanity - Function to call to check that the sensor value is sane (may be NULL)
 * @returns Number of actuators added so far
 */
int Sensor_Add(const char * name, int user_id, ReadFn read, InitFn init, CleanFn cleanup, SanityFn sanity)
{
	if (++g_num_sensors > SENSORS_MAX)
	{
		Fatal("Too many sensors; Increase SENSORS_MAX from %d in sensor.h and recompile", SENSORS_MAX);
		// We could design the program to use realloc(3)
		// But since someone who adds a new sensor has to recompile the program anyway...
	}
	Sensor * s = &(g_sensors[g_num_sensors-1]);

	s->id = g_num_sensors-1;
	s->user_id = user_id;
	Data_Init(&(s->data_file));
	s->name = name;
	s->read = read; // Set read function
	s->init = init; // Set init function

	// Start by averaging values taken over a second
	DOUBLE_TO_TIMEVAL(1, &(s->sample_time));
	s->averages = 1;
	s->num_read = 0;

	// Set sanity function
	s->sanity = sanity;

	if (init != NULL)
	{
		if (!init(name, user_id))
			Fatal("Couldn't init sensor %s", name);
	}

	s->current_data.time_stamp = 0;
	s->current_data.value = 0;
	s->averaged_data.time_stamp = 0;
	s->averaged_data.value = 0;
	return g_num_sensors;
}

/**
 * Initialise all sensors used by the program
 * TODO: Edit this to add any extra sensors you need
 * TODO: Edit the includes as well
 */
#include "sensors/resource.h"
#include "sensors/strain.h"
#include "sensors/pressure.h"
#include "sensors/dilatometer.h"
#include "sensors/microphone.h"
void Sensor_Init()
{
	//Sensor_Add("cpu_stime", RESOURCE_CPU_SYS, Resource_Read, NULL, NULL, NULL);	
	//Sensor_Add("cpu_utime", RESOURCE_CPU_USER, Resource_Read, NULL, NULL, NULL);	
	Sensor_Add("pressure_high0", PRES_HIGH0, Pressure_Read, Pressure_Init, Pressure_Cleanup, NULL);
	Sensor_Add("pressure_high1", PRES_HIGH1, Pressure_Read, Pressure_Init, Pressure_Cleanup, NULL);
	Sensor_Add("pressure_low0", PRES_LOW0, Pressure_Read, Pressure_Init, Pressure_Cleanup, NULL);
	//Sensor_Add("../testing/count.py", 0, Piped_Read, Piped_Init, Piped_Cleanup, 1e50,-1e50,1e50,-1e50);
	//Sensor_Add("strain0_endhoop", STRAIN0, Strain_Read, Strain_Init, Strain_Cleanup, Strain_Sanity);
	//Sensor_Add("strain1_endlong", STRAIN1, Strain_Read, Strain_Init, Strain_Cleanup, Strain_Sanity);
	//Sensor_Add("strain2_midhoop", STRAIN2, Strain_Read, Strain_Init, Strain_Cleanup, Strain_Sanity);
	//Sensor_Add("strain3_midlong", STRAIN3, Strain_Read, Strain_Init, Strain_Cleanup, Strain_Sanity);

	//Sensor_Add("microphone", 0, Microphone_Read, Microphone_Init, Microphone_Cleanup, Microphone_Sanity);
	//Sensor_Add("pressure0", PRESSURE0, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("pressure1", PRESSURE1, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("pressure_feedback", PRESSURE_FEEDBACK, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("enclosure", ENCLOSURE, Enclosure_Read, Enclosure_Init, 1,1,1,1); // Does not exist...

	//NOTE: DO NOT ENABLE DILATOMETER WITHOUT FURTHER TESTING; CAUSES SEGFAULTS
	//Sensor_Add("dilatometer0", 0, Dilatometer_Read, Dilatometer_Init, Dilatometer_Cleanup, NULL);
	//Sensor_Add("dilatometer1",1, Dilatometer_Read, Dilatometer_Init, Dilatometer_Cleanup, NULL);
}

/**
 * Cleanup all sensors
 */
void Sensor_Cleanup()
{
	for (int i = 0; i < g_num_sensors; ++i)
	{
		Sensor * s = g_sensors+i;
		if (s->cleanup != NULL)
			s->cleanup(s->user_id);
	}
	g_num_sensors = 0;
}

/**
 * Sets the sensor to the desired control mode. No checks are
 * done to see if setting to the desired mode will conflict with
 * the current mode - the caller must guarantee this itself.
 * @param s The sensor whose mode is to be changed
 * @param mode The mode to be changed to
 * @param arg An argument specific to the mode to be set. 
 *            e.g for CONTROL_START it represents the experiment name.
 */
void Sensor_SetMode(Sensor * s, ControlModes mode, void * arg)
{
	switch(mode)
	{
		case CONTROL_START:
			{
				// Set filename
				char filename[BUFSIZ];
				const char *experiment_path = (const char*) arg;
				int ret;

				ret = snprintf(filename, BUFSIZ, "%s/sensor_%d", experiment_path, s->id);

				if (ret >= BUFSIZ) 
				{
					Fatal("Experiment path \"%s\" too long (%d, limit %d)",
							experiment_path, ret, BUFSIZ);
				}

				Log(LOGDEBUG, "Sensor %d with DataFile \"%s\"", s->id, filename);
				// Open DataFile
				Data_Open(&(s->data_file), filename);
			}
		case CONTROL_RESUME: //Case fallthrough, no break before
			{
				int ret;
				s->activated = true; // Don't forget this!

				// Create the thread
				ret = pthread_create(&(s->thread), NULL, Sensor_Loop, (void*)(s));
				if (ret != 0)
				{
					Fatal("Failed to create Sensor_Loop for Sensor %d", s->id);
				}

				Log(LOGDEBUG, "Resuming sensor %d", s->id);
			}
		break;

		case CONTROL_EMERGENCY:
		case CONTROL_PAUSE:
			s->activated = false;
			pthread_join(s->thread, NULL);
			Log(LOGDEBUG, "Paused sensor %d", s->id);
		break;
		
		case CONTROL_STOP:
			if (s->activated) //May have been paused before
			{
				s->activated = false;
				pthread_join(s->thread, NULL);
			}

			Data_Close(&(s->data_file)); // Close DataFile
			Log(LOGDEBUG, "Stopped sensor %d", s->id);
		break;
		default:
			Fatal("Unknown control mode: %d", mode);
	}
}

/**
 * Sets all sensors to the desired mode. 
 * @see Sensor_SetMode for more information.
 * @param mode The mode to be changed to
 * @param arg An argument specific to the mode to be set.
 */
void Sensor_SetModeAll(ControlModes mode, void * arg)
{
	if (mode == CONTROL_START)
		Sensor_Init();
	for (int i = 0; i < g_num_sensors; i++)
		Sensor_SetMode(&g_sensors[i], mode, arg);
	if (mode == CONTROL_STOP)
		Sensor_Cleanup();
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
	while (s->activated)
	{
		
		bool success = s->read(s->user_id, &(s->current_data.value));

		struct timespec t;
		clock_gettime(CLOCK_MONOTONIC, &t);
		s->current_data.time_stamp = TIMEVAL_DIFF(t, *Control_GetStartTime());	
		
		if (success)
		{
			if (s->sanity != NULL)
			{
				if (!s->sanity(s->user_id, s->current_data.value))
				{
					Fatal("Sensor %s (%d,%d) reads unsafe value", s->name, s->id, s->user_id);
				}
			}
			s->averaged_data.time_stamp += s->current_data.time_stamp;
			s->averaged_data.value = s->current_data.value;
			
			if (++(s->num_read) >= s->averages)
			{
				s->averaged_data.time_stamp /= s->averages;
				s->averaged_data.value /= s->averages;
				Data_Save(&(s->data_file), &(s->averaged_data), 1); // Record it
				s->num_read = 0;
				s->averaged_data.time_stamp = 0;
				s->averaged_data.value = 0;
			}
		}
		else
		{
			// Silence because strain sensors fail ~50% of the time :S
			//Log(LOGWARN, "Failed to read sensor %s (%d,%d)", s->name, s->id,s->user_id);
		}


		clock_nanosleep(CLOCK_MONOTONIC, 0, &(s->sample_time), NULL);
		
	}
	
	// Needed to keep pthreads happy
	Log(LOGDEBUG, "Sensor %s (%d,%d) finished", s->name,s->id,s->user_id);
	return NULL;
}

/**
 * Get a Sensor given its name
 * @returns Sensor with the given name, NULL if there isn't one
 */
Sensor * Sensor_Identify(const char * name)
{	
	for (int i = 0; i < g_num_sensors; ++i)
	{
		if (strcmp(g_sensors[i].name, name) == 0)
			return &(g_sensors[i]);
	}
	return NULL;
}

/**
 * Helper: Begin sensor response in a given format
 * @param context - the FCGIContext
 * @param s - Sensor to begin the response for
 * @param format - Format
 */
void Sensor_BeginResponse(FCGIContext * context, Sensor * s, DataFormat format)
{
	// Begin response
	switch (format)
	{
		case JSON:
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONLong("id", s->id);
			FCGI_JSONLong("user_id", s->user_id); //NOTE: Might not want to expose this?
			FCGI_JSONPair("name", s->name);
			break;
		default:
			FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
			break;
	}
}

/**
 * Helper: End sensor response in a given format
 * @param context - the FCGIContext
 * @param s - Sensor to end the response for
 * @param format - Format
 */
void Sensor_EndResponse(FCGIContext * context, Sensor * s, DataFormat format)
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
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	double current_time = TIMEVAL_DIFF(now, *Control_GetStartTime());
	int id = 0;
	const char * name = "";
	double start_time = 0;
	double end_time = current_time;
	const char * fmt_str;
	double sample_s = 0;

	// key/value pairs
	FCGIValue values[] = {
		{"id", &id, FCGI_INT_T}, 
		{"name", &name, FCGI_STRING_T},
		{"format", &fmt_str, FCGI_STRING_T}, 
		{"start_time", &start_time, FCGI_DOUBLE_T}, 
		{"end_time", &end_time, FCGI_DOUBLE_T},
		{"sample_s", &sample_s, FCGI_DOUBLE_T}
	};

	// enum to avoid the use of magic numbers
	typedef enum {
		ID,
		NAME,
		FORMAT,
		START_TIME,
		END_TIME,
		SAMPLE_S
	} SensorParams;
	
	// Fill values appropriately
	if (!FCGI_ParseRequest(context, params, values, sizeof(values)/sizeof(FCGIValue)))
	{
		// Error occured; FCGI_RejectJSON already called
		return;
	}

	Sensor * s = NULL;
	if (FCGI_RECEIVED(values[NAME].flags))
	{
		if (FCGI_RECEIVED(values[ID].flags))
		{
			FCGI_RejectJSON(context, "Can't supply both sensor id and name");
			return;
		}
		s = Sensor_Identify(name);
		if (s == NULL)
		{
			FCGI_RejectJSON(context, "Unknown sensor name");
			return;
		}
	}
	else if (!FCGI_RECEIVED(values[ID].flags))
	{
		FCGI_RejectJSON(context, "No sensor id or name supplied");
		return;
	}
	else if (id < 0 || id >= g_num_sensors)
	{
		FCGI_RejectJSON(context, "Invalid sensor id");
		return;
	}
	else
	{
		s = &(g_sensors[id]);
	}

	// Adjust sample rate if necessary
	if (FCGI_RECEIVED(values[SAMPLE_S].flags))
	{
		if (sample_s < 0)
		{
			FCGI_RejectJSON(context, "Negative sampling speed!");
			return;
		}		
		DOUBLE_TO_TIMEVAL(sample_s, &(s->sample_time));
	}
	
	
	DataFormat format = Data_GetFormat(&(values[FORMAT]));

	// Begin response
	Sensor_BeginResponse(context, s, format);

	// Print Data
	Data_Handler(&(s->data_file), &(values[START_TIME]), &(values[END_TIME]), format, current_time);
	
	// Finish response
	Sensor_EndResponse(context, s, format);

}

/**
 * Get the Name of a Sensor
 * @param id - ID number
 */
const char * Sensor_GetName(int id)
{
	return g_sensors[id].name;
}

/**
 * Returns the last DataPoint that is currently available.
 * @param id - The sensor ID for which to retrieve data from
 * @return The last DataPoint
 */
DataPoint Sensor_LastData(int id)
{
	Sensor * s = &(g_sensors[id]);
	return s->current_data;
}


