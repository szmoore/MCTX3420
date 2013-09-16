/**
 * @file actuator.c
 * @purpose Implementation of Actuator related functionality
 */

#include "actuator.h"
#include "options.h"

/** Array of Actuators (global to this file) initialised by Actuator_Init **/
static Actuator g_actuators[NUMACTUATORS];

/** Human readable names for the Actuators **/
const char * g_actuator_names[NUMACTUATORS] = {	
	"actuator_test0", "actuator_test1"
};

/**
 * One off initialisation of *all* Actuators
 */
void Actuator_Init()
{
	for (int i = 0; i < NUMACTUATORS; ++i)
	{
		g_actuators[i].id = i;
		Data_Init(&(g_actuators[i].data_file));
		pthread_mutex_init(&(g_actuators[i].mutex), NULL);
	}
}

/**
 * Start an Actuator
 * @param a - The Actuator to start
 * @param experiment_name - Prepended to DataFile filename
 */
void Actuator_Start(Actuator * a, const char * experiment_name)
{
	// Set filename
	char filename[BUFSIZ];
	if (sprintf(filename, "%s_a%d", experiment_name, a->id) >= BUFSIZ)
	{
		Fatal("Experiment name \"%s\" too long (>%d)", experiment_name, BUFSIZ);
	}

	Log(LOGDEBUG, "Actuator %d with DataFile \"%s\"", a->id, filename);
	// Open DataFile
	Data_Open(&(a->data_file), filename);

	a->activated = true; // Don't forget this
	
	a->control_changed = false;

	// Create the thread
	pthread_create(&(a->thread), NULL, Actuator_Loop, (void*)(a));
}

/**
 * Stop an Actuator
 * @param s - The Actuator to stop
 */
void Actuator_Stop(Actuator * a)
{
	// Stop
	a->activated = false;
	Actuator_SetControl(a, NULL);
	pthread_join(a->thread, NULL); // Wait for thread to exit
	Data_Close(&(a->data_file)); // Close DataFile

}

/**
 * Stop all Actuators
 */
void Actuator_StopAll()
{
	for (int i = 0; i < NUMACTUATORS; ++i)
		Actuator_Stop(g_actuators+i);
}

/**
 * Start all Actuators
 */
void Actuator_StartAll(const char * experiment_name)
{
	for (int i = 0; i < NUMACTUATORS; ++i)
		Actuator_Start(g_actuators+i, experiment_name);
}

/**
 * Actuator control thread
 * @param arg - Cast to an Actuator*
 * @returns NULL to keep pthreads happy
 */
void * Actuator_Loop(void * arg)
{
	Actuator * a = (Actuator*)(arg);
	
	// Loop until stopped
	while (a->activated)
	{
		pthread_mutex_lock(&(a->mutex));
		while (!a->control_changed)
		{
			pthread_cond_wait(&(a->cond), &(a->mutex));
		}
		a->control_changed = false;
		pthread_mutex_unlock(&(a->mutex));
		if (!a->activated)
			break;
		Log(LOGDEBUG, "About to Setvalue");
		Actuator_SetValue(a, a->control.value);
	}

	//TODO: Cleanup?
	
	// Keep pthreads happy
	return NULL;
}

/**
 * Set an Actuators control variable
 * @param a - Actuator to control 
 * @param c - Control to set to
 */
void Actuator_SetControl(Actuator * a, ActuatorControl * c)
{
	pthread_mutex_lock(&(a->mutex));
	if (c != NULL)
		a->control = *c;
	a->control_changed = true;
	Log(LOGDEBUG, "About to broadcast");
	pthread_cond_broadcast(&(a->cond));
	pthread_mutex_unlock(&(a->mutex));
	
}

/**
 * Set an Actuator value
 * @param a - The Actuator
 * @param value - The value to set
 */
void Actuator_SetValue(Actuator * a, double value)
{
	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);

	DataPoint d = {TIMEVAL_DIFF(t, g_options.start_time), value};
	Log(LOGDEBUG, "id: %d", a->id);
	//TODO: Set actuator
	switch (a->id)
	{
		case ACTUATOR_TEST0:	
			{//LED actuator test code, should blink onboard LED next to Ethernet port
				FILE *LEDHandle = NULL;		//code reference: http://learnbuildshare.wordpress.com/2013/05/19/beaglebone-black-controlling-user-leds-using-c/
				char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr3/brightness";
				int val = (!!(int)value);
				Log(LOGDEBUG, "Val: %d", val);
				if(val == 1) {
					if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL) {
						fwrite("1", sizeof(char), 1, LEDHandle);
						fclose(LEDHandle);
					} else perror("fail");
				}
				else if(val == 0) {
					if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL) {
						fwrite("0", sizeof(char), 1, LEDHandle);
						fclose(LEDHandle);
					}
				}
				else perror("Pin value should be 1 or 0");
			}
			break;
		case ACTUATOR_TEST1:
			break;
	}

	Log(LOGDEBUG, "Actuator %s set to %f", g_actuator_names[a->id], value);

	// Record the value
	Data_Save(&(a->data_file), &d, 1);
}


/**
 * Helper: Begin Actuator response in a given format
 * @param context - the FCGIContext
 * @param format - Format
 * @param id - ID of Actuator
 */
void Actuator_BeginResponse(FCGIContext * context, ActuatorId id, DataFormat format)
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
 * Helper: End Actuator response in a given format
 * @param context - the FCGIContext
 * @param id - ID of the Actuator
 * @param format - Format
 */
void Actuator_EndResponse(FCGIContext * context, ActuatorId id, DataFormat format)
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
 * Handle a request for an Actuator
 * @param context - FCGI context
 * @param params - Parameters passed
 */
void Actuator_Handler(FCGIContext * context, char * params)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	double current_time = TIMEVAL_DIFF(now, g_options.start_time);
	int id = 0;
	double set = 0;
	double start_time = 0;
	double end_time = current_time;
	char * fmt_str;

	// key/value pairs
	FCGIValue values[] = {
		{"id", &id, FCGI_REQUIRED(FCGI_INT_T)}, 
		{"set", &set, FCGI_DOUBLE_T},
		{"start_time", &start_time, FCGI_DOUBLE_T},
		{"end_time", &end_time, FCGI_DOUBLE_T},
		{"format", &fmt_str, FCGI_STRING_T}
	};

	// enum to avoid the use of magic numbers
	typedef enum {
		ID,
		SET,
		START_TIME,
		END_TIME,
		FORMAT
	} ActuatorParams;
	
	// Fill values appropriately
	if (!FCGI_ParseRequest(context, params, values, sizeof(values)/sizeof(FCGIValue)))
	{
		// Error occured; FCGI_RejectJSON already called
		return;
	}	

	// Get the Actuator identified
	Actuator * a = NULL;
	if (id < 0 || id >= NUMACTUATORS)
	{
		FCGI_RejectJSON(context, "Invalid Actuator id");
		return;
	}
	
	a = g_actuators+id;

	DataFormat format = Data_GetFormat(&(values[FORMAT]));

	// Begin response
	Actuator_BeginResponse(context, id, format);

	// Set?
	if (FCGI_RECEIVED(values[SET].flags))
	{
		if (format == JSON)
			FCGI_JSONDouble("set", set);
	
		ActuatorControl c;
		c.value = set;

		Actuator_SetControl(a, &c);
	}

	// Print Data
	Data_Handler(&(a->data_file), &(values[START_TIME]), &(values[END_TIME]), format, current_time);
	
	// Finish response
	Actuator_EndResponse(context, id, format);
}
