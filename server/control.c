/**
 * @file control.c
 * @brief Handles all client control requests (admin related)
 */
#include "common.h"
#include "control.h"
#include "sensor.h"
#include "actuator.h"

typedef struct ControlData {
	ControlModes current_mode;
	pthread_mutex_t mutex;
	struct timeval start_time;
} ControlData;

ControlData g_controls = {CONTROL_STOP, PTHREAD_MUTEX_INITIALIZER, {0}};

bool PathExists(const char *path) 
{
	FILE *fp = fopen(path, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

/**
 * System control handler. This covers high-level control, including
 * admin related functions and starting/stopping experiments..
 * @param context The context to work in
 * @param params The input parameters
 */
void Control_Handler(FCGIContext *context, char *params) {
	const char *action = "";
	const char *name = "";
	bool force = false;
	ControlModes desired_mode;



	// Login/auth now handled entirely in fastcgi.c and login.c
	//TODO: Need to not have the ability for any user to stop someone else' experiment...
	// (achieve by storing the username of the person running the current experiment, even when they log out?)
	// (Our program should only realisitically support a single experiment at a time, so that should be sufficient)
	FCGIValue values[4] = {
		{"action", &action, FCGI_REQUIRED(FCGI_STRING_T)},
		{"force", &force, FCGI_BOOL_T},
		{"name", &name, FCGI_STRING_T}
	};

	if (!FCGI_ParseRequest(context, params, values, 4))
		return;
	
	if (!strcmp(action, "lock")) {
		FCGI_LockControl(context, force);
		return;
	} else if (!strcmp(action, "emergency")) {
		desired_mode = CONTROL_EMERGENCY;
	}
	else if (!strcmp(action, "release")) {
		FCGI_ReleaseControl(context);
	} else if (!strcmp(action, "start")) {
		desired_mode = CONTROL_START;
	} else if (!strcmp(action, "pause")) {
		desired_mode = CONTROL_PAUSE;
	} else if (!strcmp(action, "resume")) {
		desired_mode = CONTROL_RESUME;
	} else if (!strcmp(action, "stop")) {
		desired_mode = CONTROL_STOP; 
	} else {
		FCGI_RejectJSON(context, "Unknown action specified.");
		return;
	}
	
	void *arg = NULL;
	if (desired_mode == CONTROL_START) {
		if (PathExists(name) && !force) {
			FCGI_RejectJSON(context, "An experiment with that name already exists.");
			return;
		}

		arg = (void*)name;
	}

	const char *ret;
	if ((ret = Control_SetMode(desired_mode, arg)) != NULL) {
		FCGI_RejectJSON(context, ret);
	} else {
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("description", "ok");
		FCGI_EndJSON();
	}
}

/**
 * Sets the mode to the desired mode, if possible.
 * @param desired_mode The mode to be set to
 * @param arg An argument specific to the mode to be set. 
 * @return NULL on success, an error message on failure.
 */
const char* Control_SetMode(ControlModes desired_mode, void * arg)
{
	const char *ret = NULL;

	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.current_mode == desired_mode)
		ret = "Already in the desired mode.";
	else if (g_controls.current_mode == CONTROL_EMERGENCY && desired_mode != CONTROL_STOP)
		ret = "In emergency mode. You must stop before continuing.";
	else switch (desired_mode) {
		case CONTROL_START:
			if (g_controls.current_mode == CONTROL_STOP) {
				const char * name = arg;
				if (!*name)
					ret = "An experiment name must be specified";
				else if (strpbrk(name, INVALID_CHARACTERS))
					ret = "The experiment name must not contain: " INVALID_CHARACTERS_JSON;
				else {
					FILE *fp = fopen((const char*) arg, "a");
					if (fp) {
						fclose(fp);
						gettimeofday(&(g_controls.start_time), NULL);
					} else
						ret = "Cannot open experiment name marker";
				}
			} else 
				ret = "Cannot start when not in a stopped state.";
		break;
		case CONTROL_PAUSE:
			if (g_controls.current_mode != CONTROL_START)
				ret = "Cannot pause when not in a running state.";
		break;
		case CONTROL_RESUME:
			if (g_controls.current_mode != CONTROL_PAUSE)
				ret = "Cannot resume when not in a paused state.";
		break;
		case CONTROL_EMERGENCY:
			if (g_controls.current_mode != CONTROL_START) //pfft
				ret = "Not running so how can there be an emergency.";
		break;
		default:
		break;
	}
	
	if (ret == NULL) {
		Actuator_SetModeAll(desired_mode, arg);
		Sensor_SetModeAll(desired_mode, arg);
		if (desired_mode != CONTROL_RESUME)
			g_controls.current_mode = desired_mode;
		else
			g_controls.current_mode = CONTROL_START;
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
}

/**
 * Gets a string representation of the current mode
 * @param mode The mode to get a string representation of
 * @return The string representation of the mode
 */
const char * Control_GetModeName() {
	const char * ret = "Unknown";

	switch (g_controls.current_mode) {
		case CONTROL_START: ret = "Running"; break;
		case CONTROL_PAUSE: ret = "Paused"; break;
		case CONTROL_RESUME: ret = "Resumed"; break;
		case CONTROL_STOP: ret = "Stopped"; break;
		case CONTROL_EMERGENCY: ret = "Emergency mode"; break;
	}
	return ret;
}

/**
 * Gets the start time for the current experiment
 * @return the start time
 */
const struct timeval* Control_GetStartTime() {
	return &g_controls.start_time;
}
