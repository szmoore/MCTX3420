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
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{5000,0,5000,0},
	{1, 1, 1, 1}
};

/** Human readable names for the sensors **/
const char * g_sensor_names[NUMSENSORS] = {	
	"strain0",
	"strain1",
	"strain2",
	"strain3",
	"pressure0",
	"pressure1",
	"pressure_feedback",
	"microphone",
	"enclosure"
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



	// Get the required ADCs
	ADC_Export(ADC0); // Strain gauges x 4
	ADC_Export(ADC1); // Pressure sensor 1
	ADC_Export(ADC2); // Pressure sensor 2
	// ADC3 still unused (!?)
	ADC_Export(ADC4); // Pressure regulator feedback(?) signal
	ADC_Export(ADC5); // Microphone

	// Get GPIO pins //TODO: Confirm pins used with Electronics Team
	GPIO_Export(GPIO0_30); // Mux A (strain 1)
	GPIO_Set(GPIO0_30, false);
	GPIO_Export(GPIO1_28); // Mux B (strain 2)
	GPIO_Set(GPIO1_28, false);
	GPIO_Export(GPIO0_31); // Mux C (strain 3)
	GPIO_Set(GPIO0_31, false);
	GPIO_Export(GPIO1_16); // Mux D (strain 4)
	GPIO_Set(GPIO1_16, false);

	GPIO_Export(GPIO0_31); // Enclosure switch
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
		Log(LOGERR, "Sensor %s at %f is above or below its safety value of %f or %f\n", g_sensor_names[id],value, thresholds[id].max_error, thresholds[id].min_error);
		//new function that stops actuators?
		//Control_SetMode(CONTROL_EMERGENCY, NULL)
	}
	else if( value > thresholds[id].max_warn || value < thresholds[id].min_warn)
	{
		Log(LOGWARN, "Sensor %s at %f is above or below its warning value of %f or %f\n", g_sensor_names[id],value,thresholds[id].max_warn, thresholds[id].min_warn);	
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
	


	static bool result = true;

	//TODO: Remove this, code should be refactored to not use so many threads
	// Although... if it works, it works...
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

	pthread_mutex_lock(&mutex); //TODO: Reduce the critical section

	usleep(10);

	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);
	d->time_stamp = TIMEVAL_DIFF(t, *Control_GetStartTime());	
	
	// Read value based on Sensor Id
	int value; bool success = true;
	//TODO: Can probably do this nicer than a switch (define a function pointer for each sensor)
	//		Can probably make the whole sensor thing a lot nicer with a linked list of sensors...
	//		(Then to add more sensors to the software, someone just writes an appropriate read function and calls Sensor_Add(...) at init)
	//		(I will do this. Don't do it before I get a chance, I don't trust you :P)
	switch (s->id)
	{
		//TODO: Strain gauges should have their own critical section, rest of sensors probably don't need to be in a critical section
		case STRAIN0:
			success &= GPIO_Set(GPIO0_30, true);
			success &= ADC_Read(ADC0);
			success &= GPIO_Set(GPIO0_30, false);
			if (!success)
				Fatal("Error reading strain gauge 0");
			break;
		case STRAIN1:
			success &= GPIO_Set(GPIO1_28, true);
			success &= ADC_Read(ADC0);
			success &= GPIO_Set(GPIO1_28, false);
			if (!success)
				Fatal("Error reading strain gauge 1");
			break;
		case STRAIN2:
			success &= GPIO_Set(GPIO0_31, true);
			success &= ADC_Read(ADC0);
			success &= GPIO_Set(GPIO0_31, false);
		case STRAIN3:
			success &= GPIO_Set(GPIO1_16, true);
			success &= ADC_Read(ADC0);
			success &= GPIO_Set(GPIO1_16, false);
			if (!success)
				Fatal("Error reading strain gauge 2");	
			break;		
		case PRESSURE0:
			success &= ADC_Read(ADC1, &value);
			break;
		case PRESSURE1:
			success &= ADC_Read(ADC5, &value);
			break;
		case PRESSURE_FEEDBACK:
			success &= ADC_Read(ADC4, &value);
			break;
		case MICROPHONE:
			success &= ADC_Read(ADC2, &value);
			break;
		case ENCLOSURE:
		{
			bool why_do_i_need_to_do_this = false;
			success &= GPIO_Read(GPIO0_31, &why_do_i_need_to_do_this);
			value = (int)why_do_i_need_to_do_this;
			break;
		}
		case DILATOMETER:
		{
			// Will definitely cause issues included in the same critical section as ADC reads
			// (since it will be the longest sensor to sample, everything else will have to keep waiting on it)
			value = 0;
			break;
		}
		
	}	

	d->value = (double)(value); //TODO: Calibration? Or do calibration in GUI

	pthread_mutex_unlock(&mutex); //TODO: Reduce the critical section
	

	// Perform sanity check based on Sensor's ID and the DataPoint
	Sensor_CheckData(s->id, d->value);

	// Update latest DataPoint if necessary
	
	if (result)
	{
		s->newest_data.time_stamp = d->time_stamp;
		s->newest_data.value = d->value;
	}

#ifdef _BBB
	//Not all cases have usleep, easiest here.
	//TODO: May want to add a control option to adjust the sampling rate for each sensor?
	//		Also, we can get a more accurate sampling rate if instead of a fixed sleep, we calculate how long to sleep each time.
	usleep(100000);
#endif

	/*
	if (success)
		Log(LOGDEBUG, "Successfully read sensor %d (for once)", s->id);
	else
		Log(LOGDEBUG, "Failed to read sensor %d (again)", s->id);
	*/
	return result && success;
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



