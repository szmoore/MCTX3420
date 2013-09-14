/**
 * @file control.c
 * @brief Handles all client control requests (admin related)
 */
#include "common.h"
#include "control.h"
#include "sensor.h"
#include "actuator.h"

typedef enum ControlState {
	STATE_STOPPED,
	STATE_PAUSED,
	STATE_RUNNING
} ControlState;

typedef struct ControlData {
	ControlState state;
	pthread_mutex_t mutex;
	struct timeval start_time;
} ControlData;

ControlData g_controls = {STATE_STOPPED, PTHREAD_MUTEX_INITIALIZER, {0}};

/**
 * System control handler. This covers high-level control, including
 * admin related functions and starting/stopping experiments..
 * @param context The context to work in
 * @param params The input parameters
 */
void Control_Handler(FCGIContext *context, char *params) {
	const char *action, *key = "", *name = "";
	bool force = false;

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
	} else if (FCGI_HasControl(context, key)) {
		if (!strcmp(action, "release")) {
			FCGI_ReleaseControl(context);
		} else if (!strcmp(action, "start")) {
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONPair("description", "start");
			FCGI_EndJSON();
		} else if (!strcmp(action, "pause")) {
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONPair("description", "stop");
			FCGI_EndJSON();
		} else if (!strcmp(action, "end")) {
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONPair("description", "stop");
			FCGI_EndJSON();
		} else {
			FCGI_RejectJSON(context, "Unknown action specified.");
		}
	} else {
		FCGI_RejectJSONEx(context, STATUS_UNAUTHORIZED, 
			"Invalid control key specified.");
	}
}

bool Control_Start(const char *experiment_name) {
	bool ret = false;

	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_STOPPED) {
		gettimeofday(&(g_controls.start_time), NULL);
		Sensor_StartAll(experiment_name);
		Actuator_StartAll(experiment_name);
		g_controls.state = STATE_RUNNING;
		ret = true;
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
}


void Control_Pause() {
	pthread_mutex_lock(&(g_controls.mutex));
	pthread_mutex_unlock(&(g_controls.mutex));
}

bool Control_Stop() {
	bool ret = false;

	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state != STATE_STOPPED) {
		Actuator_StopAll();
		Sensor_StopAll();
		g_controls.state = STATE_STOPPED;
		ret = true;
	}
	pthread_mutex_unlock(&(g_controls.mutex));	
	return ret;
}

bool Control_Lock() {
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_RUNNING)
		return true;
	pthread_mutex_unlock(&(g_controls.mutex));
	return false;
}

void Control_Unlock() {
	pthread_mutex_unlock(&(g_controls.mutex));
}

const struct timeval* Control_GetStartTime() {
	return &g_controls.start_time;
}