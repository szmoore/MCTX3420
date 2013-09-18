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

static bool PathExists(const char *path) {
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
	const char *action, *key = "", *name = "";
	bool force = false;
	ControlModes desired_mode;

	FCGIValue values[4] = {
		{"action", &action, FCGI_REQUIRED(FCGI_STRING_T)},
		{"key", &key, FCGI_STRING_T},
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
	} else if (!strcmp(action, "query")) {
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("state", Control_GetModeName(Control_GetMode()));
		FCGI_EndJSON();
		return;
	} else if (FCGI_HasControl(context, key)) {
		if (!strcmp(action, "release")) {
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
	} else {
		FCGI_RejectJSONEx(context, STATUS_UNAUTHORIZED, 
			"Invalid control key specified.");
		return;
	}

	void *arg = NULL;
	if (desired_mode == CONTROL_START) {
		int len = strlen(name);
		if (len <= 0) {
			FCGI_RejectJSON(context, "An experiment name must be specified.");
			return;
		} else if (PathExists(name) && !force) {
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
	if (g_controls.current_mode == CONTROL_EMERGENCY && desired_mode != CONTROL_STOP) {
		ret = "In emergency mode. Stop before doing anything else.";
	} else if (g_controls.current_mode == desired_mode) {
		ret = "Already in desired mode.";
	} else if (desired_mode == CONTROL_START) {
		if (g_controls.current_mode == CONTROL_STOP) {
			//TODO Sanitise name (ensure it contains no special chars eg \ / .. . 
			FILE *fp = fopen((const char*) arg, "a");
			if (fp) {
				fclose(fp);
				gettimeofday(&(g_controls.start_time), NULL);
			} else {
				ret = "Cannot open experiment name marker";
			}
		} else {
			ret = "Cannot start when not in a stopped state.";
		}
	} else if (desired_mode == CONTROL_RESUME) {
		if (g_controls.current_mode != CONTROL_PAUSE)
			ret = "Cannot resume when not in a paused state.";
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
 * Gets the current mode.
 * @return The current mode
 */
ControlModes Control_GetMode() {
	return g_controls.current_mode;
}

/**
 * Gets a string representation of a mode
 * @param mode The mode to get a string representation of
 * @return The string representation of the mode
 */
const char * Control_GetModeName(ControlModes mode) {
	const char * ret = "Unknown";

	switch (mode) {
		case CONTROL_START: ret = "Running"; break;
		case CONTROL_PAUSE: ret = "Paused"; break;
		case CONTROL_RESUME: ret = "Resumed"; break;
		case CONTROL_STOP: ret = "Stopped"; break;
		case CONTROL_EMERGENCY: ret = "Emergency mode"; break;
	}
	return ret;
}

/*
bool Control_Lock() {
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_RUNNING || g_controls.state == STATE_PAUSED)
		return true;
	pthread_mutex_unlock(&(g_controls.mutex));
	return false;
}

void Control_Unlock() {
	pthread_mutex_unlock(&(g_controls.mutex));
}*/

/**
 * Gets the start time for the current experiment
 * @return the start time
 */
const struct timeval* Control_GetStartTime() {
	return &g_controls.start_time;
}