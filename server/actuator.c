/**
 * @file actuator.c
 * @brief Implementation of Actuator related functionality
 */

#include "actuator.h"
#include "options.h"
// Files containing GPIO and PWM definitions
#include "bbb_pin.h"




/** Number of actuators **/
int g_num_actuators = 0;

/** Array of Actuators (global to this file) initialised by Actuator_Init **/
static Actuator g_actuators[ACTUATORS_MAX];
/** 
 * Add and initialise an Actuator
 * @param name - Human readable name of the actuator
 * @param read - Function to call whenever the actuator should be read
 * @param init - Function to call to initialise the actuator (may be NULL)
 * @returns Number of actuators added so far
 */
int Actuator_Add(const char * name, int user_id, SetFn set, InitFn init, CleanFn cleanup, SanityFn sanity, double initial_value)
{
	if (++g_num_actuators > ACTUATORS_MAX)
	{
		Fatal("Too many sensors; Increase ACTUATORS_MAX from %d in actuator.h and recompile", ACTUATORS_MAX);
	}
	Actuator * a = &(g_actuators[g_num_actuators-1]);
	a->id = g_num_actuators-1;
	a->user_id = user_id;
	Data_Init(&(a->data_file));
	a->name = name;
	a->set = set; // Set read function
	a->init = init; // Set init function

	a->sanity = sanity;

	pthread_mutex_init(&(a->mutex), NULL);

	if (init != NULL)
	{
		if (!init(name, user_id))
			Fatal("Couldn't initialise actuator %s", name);
	}

	Actuator_SetValue(a, initial_value, false);

	return g_num_actuators;
}


/**
 * One off initialisation of *all* Actuators
 */
#include "actuators/ledtest.h"
#include "actuators/filetest.h"
void Actuator_Init()
{
	//Actuator_Add("ledtest",0,  Ledtest_Set, NULL,NULL,NULL);
	Actuator_Add("filetest", 0, Filetest_Set, Filetest_Init, Filetest_Cleanup, Filetest_Sanity, 0);
}

/**
 * Sets the actuator to the desired mode. No checks are
 * done to see if setting to the desired mode will conflict with
 * the current mode - the caller must guarantee this itself.
 * @param a The actuator whose mode is to be changed
 * @param mode The mode to be changed to
 * @param arg An argument specific to the mode to be set. 
 *            e.g for CONTROL_START it represents the experiment name.
 */
void Actuator_SetMode(Actuator * a, ControlModes mode, void *arg)
{
	switch (mode)
	{
		case CONTROL_START:
			{
				char filename[BUFSIZ];
				const char *experiment_name = (const char*) arg;

				if (snprintf(filename, BUFSIZ, "%s_a%d", experiment_name, a->id) >= BUFSIZ)
				{
					Fatal("Experiment name \"%s\" too long (>%d)", experiment_name, BUFSIZ);
				}

				Log(LOGDEBUG, "Actuator %d with DataFile \"%s\"", a->id, filename);
				// Open DataFile
				Data_Open(&(a->data_file), filename);
			} 
		case CONTROL_RESUME:  //Case fallthrough; no break before
			{
				int ret;
				a->activated = true; // Don't forget this
				a->control_changed = false;

				ret = pthread_create(&(a->thread), NULL, Actuator_Loop, (void*)(a));
				if (ret != 0)
				{
					Fatal("Failed to create Actuator_Loop for Actuator %d", a->id);
				}

				Log(LOGDEBUG, "Resuming actuator %d", a->id);
			}
		break;

		case CONTROL_EMERGENCY: //TODO add proper case for emergency
		case CONTROL_PAUSE:
			a->activated = false;
			Actuator_SetControl(a, NULL);
			pthread_join(a->thread, NULL); // Wait for thread to exit

			Log(LOGDEBUG, "Paused actuator %d", a->id);
		break;

		break;
		case CONTROL_STOP:
			if (a->activated) //May have been paused before
			{
				a->activated = false;
				Actuator_SetControl(a, NULL);
				pthread_join(a->thread, NULL); // Wait for thread to exit	
			}
			Data_Close(&(a->data_file)); // Close DataFile
			
			Log(LOGDEBUG, "Stopped actuator %d", a->id);
		break;
		default:
			Fatal("Unknown control mode: %d", mode);
	}
}

/**
 * Sets all actuators to the desired mode. 
 * @see Actuator_SetMode for more information.
 * @param mode The mode to be changed to
 * @param arg An argument specific to the mode to be set.
 */
void Actuator_SetModeAll(ControlModes mode, void * arg)
{
	for (int i = 0; i < ACTUATORS_MAX; i++)
		Actuator_SetMode(&g_actuators[i], mode, arg);
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

		Actuator_SetValue(a, a->control.start, true);
		// Currently does discrete steps after specified time intervals

		struct timespec wait;
		DOUBLE_TO_TIMEVAL(a->control.stepsize, &wait);
		while (!a->control_changed && a->control.steps > 0 && a->activated)
		{
			clock_nanosleep(CLOCK_MONOTONIC, 0, &wait, NULL);
			a->control.start += a->control.stepsize;
			Actuator_SetValue(a, a->control.start, true);
			
			a->control.steps--;
		}
		if (a->control_changed)
			continue;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &wait, NULL);

		//TODO:
		// Note that although this loop has a sleep in it which would seem to make it hard to enforce urgent shutdowns,
		//	You can call the Actuator's cleanup function immediately (and this loop should later just exit)
		//	tl;dr This function isn't/shouldn't be responsible for the emergency Actuator stuff
		// (That should be handled by the Fatal function... at some point)
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
	pthread_cond_broadcast(&(a->cond));
	pthread_mutex_unlock(&(a->mutex));
	
}

/**
 * Set an Actuator value
 * @param a - The Actuator
 * @param value - The value to set
 */
void Actuator_SetValue(Actuator * a, double value, bool record)
{
	if (a->sanity != NULL && !a->sanity(a->user_id, value))
	{
		//ARE YOU INSANE?
		Log(LOGERR,"Insane value %lf for actuator %s", value, a->name);
		return;
	}
	if (!(a->set(a->user_id, value)))
	{
		Fatal("Failed to set actuator %s to %lf", a->name, value);
	}

	// Set time stamp
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	DataPoint d = {TIMEVAL_DIFF(t, *Control_GetStartTime()), a->last_setting.value};
	// Record value change
	if (record)
	{	
		d.time_stamp -= 1e-6;
		Data_Save(&(a->data_file), &d, 1);
		d.value = value;
		d.time_stamp += 1e-6;
		Data_Save(&(a->data_file), &d, 1);
	}
	a->last_setting = d;
}

/**
 * Helper: Begin Actuator response in a given format
 * @param context - the FCGIContext
 * @param format - Format
 * @param id - ID of Actuator
 */
void Actuator_BeginResponse(FCGIContext * context, Actuator * a, DataFormat format)
{
	// Begin response
	switch (format)
	{
		case JSON:
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONLong("id", a->id);
			FCGI_JSONLong("user_id", a->user_id); //TODO: Don't need to show this?
			FCGI_JSONPair("name", a->name);
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
void Actuator_EndResponse(FCGIContext * context, Actuator * a, DataFormat format)
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
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	double current_time = TIMEVAL_DIFF(now, *Control_GetStartTime());
	int id = 0;
	char * name = "";
	char * set = "";
	double start_time = 0;
	double end_time = current_time;
	char * fmt_str;

	// key/value pairs
	FCGIValue values[] = {
		{"id", &id, FCGI_INT_T},
		{"name", &name, FCGI_STRING_T}, 
		{"set", &set, FCGI_STRING_T},
		{"start_time", &start_time, FCGI_DOUBLE_T},
		{"end_time", &end_time, FCGI_DOUBLE_T},
		{"format", &fmt_str, FCGI_STRING_T}
	};

	// enum to avoid the use of magic numbers
	typedef enum {
		ID,
		NAME,
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

	if (FCGI_RECEIVED(values[NAME].flags))
	{
		if (FCGI_RECEIVED(values[ID].flags))
		{
			FCGI_RejectJSON(context, "Can't supply both id and name");
			return;
		}
		a = Actuator_Identify(name);
		if (a == NULL)
		{
			FCGI_RejectJSON(context, "Unknown actuator name");
			return;
		}
		
	}
	else if (!FCGI_RECEIVED(values[ID].flags))
	{
		FCGI_RejectJSON(context, "No id or name supplied");
		return;
	}
	else if (id < 0 || id >= g_num_actuators)
	{
		FCGI_RejectJSON(context, "Invalid Actuator id");
		return;
	}
	else
	{
		a = &(g_actuators[id]);
	}
	

	DataFormat format = Data_GetFormat(&(values[FORMAT]));




	if (FCGI_RECEIVED(values[SET].flags))
	{
		
	
		ActuatorControl c = {0.0, 0.0, 0.0, 0}; // Need to set default values (since we don't require them all)
		// sscanf returns the number of fields successfully read...
		int n = sscanf(set, "%lf,%lf,%lf,%d", &(c.start), &(c.stepwait), &(c.stepsize), &(c.steps)); // Set provided values in order
		if (n != 4)
		{
			//	If the user doesn't provide all 4 values, the Actuator will get set *once* using the first of the provided values
			//	(see Actuator_Loop)
			//  Not really a problem if n = 1, but maybe generate a warning for 2 <= n < 4 ?
			Log(LOGDEBUG, "Only provided %d values (expect %d) for Actuator setting", n, 4);
		}
		// SANITY CHECKS
		if (c.stepwait < 0 || c.steps < 0 || (a->sanity != NULL && !a->sanity(a->user_id, c.start)))
		{
			FCGI_RejectJSON(context, "Bad Actuator setting");
			return;
		}
		Actuator_SetControl(a, &c);
	}
	
	// Begin response
	Actuator_BeginResponse(context, a, format);
	if (format == JSON)
		FCGI_JSONPair("set", set);

	// Print Data
	Data_Handler(&(a->data_file), &(values[START_TIME]), &(values[END_TIME]), format, current_time);
	
	// Finish response
	Actuator_EndResponse(context, a, format);
}

/**
 * Get the name of an Actuator given its id
 * @param id - ID of the actuator
 * @returns The Actuator's name
 */
const char * Actuator_GetName(int id)
{
	return g_actuators[id].name;
}

/**
 * Identify an Actuator from its name string
 * @param name - The name of the Actuator
 * @returns Actuator
 */
Actuator * Actuator_Identify(const char * name)
{
	for (int i = 0; i < g_num_actuators; ++i)
	{
		if (strcmp(g_actuators[i].name, name) == 0)
			return &(g_actuators[i]);
	}
	return NULL;
}
