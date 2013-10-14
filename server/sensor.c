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
 * @param max_error - Maximum error threshold; program will exit if this is exceeded for the sensor reading
 * @param min_error - Minimum error threshold; program will exit if the sensor reading falls below this value
 * @param max_warn - Maximum warning threshold; program will log warnings if the value exceeds this threshold
 * @param min_warn - Minimum warning threshold; program will log warnings if the value falls below this threshold
 * @returns Number of actuators added so far
 */
int Sensor_Add(const char * name, int user_id, ReadFn read, InitFn init, CleanFn cleanup, double max_error, double min_error, double max_warn, double min_warn)
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
	if (init != NULL)
		init(name, user_id); // Call it

	// Start by averaging values taken over a second
	s->sample_us = 1e6;
	s->averages = 1;

	// Set warning/error thresholds
	s->thresholds.max_error = max_error;
	s->thresholds.min_error = min_error;
	s->thresholds.max_warn = max_warn;
	s->thresholds.min_warn = min_warn;

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
void Sensor_Init()
{
	Sensor_Add("cpu_stime", RESOURCE_CPU_SYS, Resource_Read, NULL, NULL, 1e50,-1e50,1e50,-1e50);	
	Sensor_Add("cpu_utime", RESOURCE_CPU_USER, Resource_Read, NULL, NULL, 1e50,-1e50,1e50,-1e50);	
	Sensor_Add("pressure_high0", PRES_HIGH0, Pressure_Read, Pressure_Init, Pressure_Cleanup, 800, 110, 800, 110);
	Sensor_Add("pressure_high1", PRES_HIGH1, Pressure_Read, Pressure_Init, Pressure_Cleanup, 800, 110, 800, 110);
	Sensor_Add("pressure_low0", PRES_LOW0, Pressure_Read, Pressure_Init, Pressure_Cleanup, 250, 110, 250, 110);
	//Sensor_Add("../testing/count.py", 0, Piped_Read, Piped_Init, Piped_Cleanup, 1e50,-1e50,1e50,-1e50);
	//Sensor_Add("strain0", STRAIN0, Strain_Read, Strain_Init, 5000,0,5000,0);
	//Sensor_Add("strain1", STRAIN1, Strain_Read, Strain_Init, 5000,0,5000,0);
	//Sensor_Add("strain2", STRAIN2, Strain_Read, Strain_Init, 5000,0,5000,0);
	//Sensor_Add("strain3", STRAIN3, Strain_Read, Strain_Init, 5000,0,5000,0);
	//Sensor_Add("pressure0", PRESSURE0, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("pressure1", PRESSURE1, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("pressure_feedback", PRESSURE_FEEDBACK, Pressure_Read, Pressure_Init, 5000,0,5000,0);
	//Sensor_Add("enclosure", ENCLOSURE, Enclosure_Read, Enclosure_Init, 1,1,1,1);
	//Sensor_Add("dilatometer", DILATOMETER, Dilatometer_Read, Dilatometer_Init, -1,-1,-1,-1);
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
				const char *experiment_name = (const char*) arg;

				if (snprintf(filename, BUFSIZ, "%s_%d", experiment_name, s->id) >= BUFSIZ)
				{
					Fatal("Experiment name \"%s\" too long (>%d)", experiment_name, BUFSIZ);
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
	for (int i = 0; i < g_num_sensors; i++)
		Sensor_SetMode(&g_sensors[i], mode, arg);
}


/**
 * Checks the sensor data for unsafe or unexpected results 
 * @param sensor_id - The ID of the sensor
 * @param value - The value from the sensor to test
 */
void Sensor_CheckData(Sensor * s, double value)
{
	if( value > s->thresholds.max_error || value < s->thresholds.min_error)
	{
		Log(LOGERR, "Sensor %s at %f is above or below its safety value of %f or %f\n",s->name,value, s->thresholds.max_error, s->thresholds.min_error);
		//new function that stops actuators?
		//Control_SetMode(CONTROL_EMERGENCY, NULL)
	}
	else if( value > s->thresholds.max_warn || value < s->thresholds.min_warn)
	{
		Log(LOGWARN, "Sensor %s at %f is above or below its warning value of %f or %f\n", s->name,value,s->thresholds.max_warn, s->thresholds.min_warn);	
	}
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
		DataPoint d;
		d.value = 0;
		bool success = s->read(s->user_id, &(d.value));

		struct timeval t;
		gettimeofday(&t, NULL);
		d.time_stamp = TIMEVAL_DIFF(t, *Control_GetStartTime());	
		
		if (success)
		{


			Sensor_CheckData(s, d.value);
			Data_Save(&(s->data_file), &d, 1); // Record it
		}
		else
			Log(LOGWARN, "Failed to read sensor %s (%d,%d)", s->name, s->id,s->user_id);

		usleep(s->sample_us);
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
 * @param id - ID of sensor
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
 * @param id - ID of the sensor
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
	struct timeval now;
	gettimeofday(&now, NULL);
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
		s->sample_us = 1e6*sample_s;
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



