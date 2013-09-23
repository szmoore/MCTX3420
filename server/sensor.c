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
static Sensor g_sensors[NUMSENSORS]; //global to this file

/** Array of sensor threshold structures defining the safety values of each sensor**/
const SensorThreshold thresholds[NUMSENSORS]= {
	//Max Safety, Min safety, Max warning, Min warning
	{1,-1,1,-1},		// ANALOG_TEST0
	{500,0,499,0},		// ANALOG_TEST1
	{5000,0,5000,0},		// ANALOG_REALTEST
	{5,-5,4,-4},		// ANALOG_FAIL0
	{1,0,1,0},			// DIGITAL_TEST0
	{1,0,1,0},			// DIGITAL_TEST1
	{1,0,1,0},			// DIGITAL_REALTEST
	{1,0,1,0}			// DIGITAL_FAIL0
};

/** Human readable names for the sensors **/
const char * g_sensor_names[NUMSENSORS] = {	
	"analog_test0", "analog_test1", 
	"analog_realtest", "analog_fail0",
	"digital_test0", "digital_test1", 
	"digital_realtest", "digital_fail0"
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
	}

	// Get the ADCs
	//ADC_Export();

	// GPIO1_28 used as a pulse for sampling
	//GPIO_Export(GPIO1_28);
	// GPIO0_30 toggled during sampling
	//GPIO_Export(GPIO0_30);
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

				if (snprintf(filename, BUFSIZ, "%s_s%d", experiment_name, s->id) >= BUFSIZ)
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
			s->newest_data.time_stamp = 0;
			s->newest_data.value = 0;
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
	for (int i = 0; i < NUMSENSORS; i++)
		Sensor_SetMode(&g_sensors[i], mode, arg);
}


/**
 * Checks the sensor data for unsafe or unexpected results 
 * @param sensor_id - The ID of the sensor
 * @param value - The value from the sensor to test
 */
void Sensor_CheckData(SensorId id, double value)
{
	if( value > thresholds[id].max_error || value < thresholds[id].min_error)
	{
		Log(LOGERR, "Sensor %s at %f is above or below its safety value of %f or %f\n", value, g_sensor_names[id],thresholds[id].max_error, thresholds[id].min_error);
		//new function that stops actuators?
		//Control_SetMode(CONTROL_EMERGENCY, NULL)
	}
	else if( value > thresholds[id].max_warn || value < thresholds[id].min_warn)
	{
		Log(LOGWARN, "Sensor %s at %f is above or below its warning value of %f or %f\n", value, g_sensor_names[id],thresholds[id].max_warn, thresholds[id].min_warn);	
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
	d->time_stamp = TIMEVAL_DIFF(t, *Control_GetStartTime());

	static bool result = true;
	
	
	// Read value based on Sensor Id
	switch (s->id)
	{
		case 2:
		{
			static bool set = false;
			
			//GPIO_Set(GPIO0_30, true);
			d->value = (double)ADC_Read(ADC0);	//ADC #0 on the Beaglebone
			//Log(LOGDEBUG, "Got value %f from ADC0", d->value);
			//GPIO_Set(GPIO0_30, false);
			set = !set;
			//GPIO_Set(GPIO1_28, set);

			usleep(100000);
			
			break;
		}
		
		default:
			d->value = rand() % 2;
			usleep(1000000);
			break;
		

		case ANALOG_TEST0:
		{
			d->value = (double)(rand() % 100) / 100;
			break;
		}
		case ANALOG_TEST1:
		{
			static int count = 0;
			count %= 500;
			d->value = count++;
			break;
		}

		case ANALOG_FAIL0:
			d->value = 0;
			//d->value = (double)(rand() % 6) * -( rand() % 2) / ( rand() % 100 + 1);
			//Gives a value between -5 and 5
			break;
		case DIGITAL_TEST0:
			d->value = t.tv_sec % 2;

			break;
		case DIGITAL_TEST1:
			d->value = (t.tv_sec+1)%2;
			break;
		case DIGITAL_REALTEST:
		{
		// Can pass pin as argument, just using 20 as an example here
		// Although since pins will be fixed, can just define it here if we need to
			//d->value = pinRead(20);	//Pin 20 on the Beaglebone
			break;
		}
		case DIGITAL_FAIL0:
			if( rand() % 100 > 98)
				d->value = 2;
			d->value = rand() % 2; 
			//Gives 0 or 1 or a 2 every 1/100 times
			break;
		//default:
		//	Fatal("Unknown sensor id: %d", s->id);
		//	break;
	}	
	

	// Perform sanity check based on Sensor's ID and the DataPoint
	Sensor_CheckData(s->id, d->value);

	// Update latest DataPoint if necessary
	
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
	while (s->activated)
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
			FCGI_JSONPair("name", g_sensor_names[id]);
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
	double current_time = TIMEVAL_DIFF(now, *Control_GetStartTime());

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



