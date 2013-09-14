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

typedef enum Mode {
	START,
	PAUSE,
	RESUME,
	STOP
} Mode;

typedef struct ControlData {
	ControlState state;
	pthread_mutex_t mutex;
	struct timeval start_time;
} ControlData;

ControlData g_controls = {STATE_STOPPED, PTHREAD_MUTEX_INITIALIZER, {0}};

static bool ExperimentExists(const char *experiment_name) {
	FILE *fp = fopen(experiment_name, "r");
	if (!fp)
		return false;
	fclose(fp);
	return true;
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
	Mode mode;

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
	} else if (FCGI_HasControl(context, key)) {
		if (!strcmp(action, "release")) {
			FCGI_ReleaseControl(context);
		} else if (!strcmp(action, "start")) {
			mode = START;
		} else if (!strcmp(action, "pause")) {
			mode = PAUSE;
		} else if (!strcmp(action, "resume")) {
			mode = RESUME;
		} else if (!strcmp(action, "stop")) {
			mode = STOP;
		} else {
			FCGI_RejectJSON(context, "Unknown action specified.");
			return;
		}
	} else {
		FCGI_RejectJSONEx(context, STATUS_UNAUTHORIZED, 
			"Invalid control key specified.");
		return;
	}

	switch(mode) {
		case START:
			if (!*name) {
				FCGI_RejectJSON(context, "An experiment name must be provided");
			} else if (ExperimentExists(name) && !force) {
				FCGI_RejectJSONEx(context, STATUS_ALREADYEXISTS,
					"An experiment with the specified name already exists.");
			} else if (!Control_Start(name)) {
				FCGI_RejectJSON(context, "An experiment is already running.");
			} else {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_EndJSON();
			}
			break;
		case PAUSE:
			if (!Control_Pause()) {
				FCGI_RejectJSON(context, "No experiment to pause.");
			} else {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_EndJSON();
			}
			break;
		case RESUME:
			if (!Control_Resume()) {
				FCGI_RejectJSON(context, "No experiment to resume.");
			} else {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_EndJSON();				
			}
			break;
		case STOP:
			if (!Control_Stop()) {
				FCGI_RejectJSON(context, "No experiment to stop.");
			} else {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_EndJSON();
			}
			break;
	}
}

bool Control_Start(const char *experiment_name) {
	bool ret = false;

	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_STOPPED) {
		FILE *fp = fopen(experiment_name, "a");
		if (fp) {
			fclose(fp);
			gettimeofday(&(g_controls.start_time), NULL);
			Sensor_StartAll(experiment_name);
			Actuator_StartAll(experiment_name);
			g_controls.state = STATE_RUNNING;
			ret = true;
		}
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
}


bool Control_Pause() {
	bool ret = false;
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_RUNNING) {
		Actuator_PauseAll();
		Sensor_PauseAll();
		g_controls.state = STATE_PAUSED;
		ret = true;
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
}

bool Control_Resume() {
	bool ret = false;
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_PAUSED) {
		Actuator_ResumeAll();
		Sensor_ResumeAll();
		g_controls.state = STATE_RUNNING;
		ret = true;
	}
	pthread_mutex_unlock(&(g_controls.mutex));
	return ret;
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
	if (g_controls.state == STATE_RUNNING || g_controls.state == STATE_PAUSED)
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