/**
 * @file control.c
 * @brief Handles all client control requests (admin related)
 */
#include "common.h"
#include "control.h"


const char * g_actuator_names[NUMACTUATORS] = {	
	"Pressure regulator", "Solenoid 1" 
};

/*
void ActuatorHandler(FCGIContext *context, ActuatorId id, const char *set_value) {
	char *ptr;
	
	switch(id) { //Add new actuators here
		case ACT_PRESSURE: //Suppose is pressure regulator. 0-700 input (kPa)
		{
			int value = strtol(set_value, &ptr, 10);
			if (*ptr == '\0' && value >= 0 && value <= 700) {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_JSONKey("description");
				FCGI_JSONValue("\"Set pressure to %d kPa!\"", value);
				FCGI_EndJSON();
			} else {
				FCGI_RejectJSONEx(context, 
					STATUS_ERROR, "Invalid pressure specified.");
			}
		} break;
		case ACT_SOLENOID1:
		{
			int value = strtol(set_value, &ptr, 10);
			if (*ptr == '\0') {
				const char *state = "off";
				if (value)
					state = "on";
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_JSONKey("description");
				FCGI_JSONValue("\"Solenoid 1 turned %s!\"", state);
				FCGI_EndJSON();
			} else {
				FCGI_RejectJSON(context, "Invalid actuator value specified");
			}
		} break;
		default:
			FCGI_RejectJSONEx(context, 
				STATUS_ERROR, "Invalid actuator id specified.");
	}
}*/

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
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state == STATE_STOPPED) {
		gettimeofday(&(g_controls.start_time), NULL);
		Sensor_StartAll(experiment_name);
		g_controls.state = STATE_RUNNING;

		pthread_mutex_unlock(&(g_controls.mutex));
		return true;
	}
	return false;
	pthread_mutex_unlock(&(g_controls.mutex));
}

void Control_Pause() {
	pthread_mutex_lock(&(g_controls.mutex));
	pthread_mutex_unlock(&(g_controls.mutex));
}

bool Control_End() {
	pthread_mutex_lock(&(g_controls.mutex));
	if (g_controls.state != STATE_STOPPED) {
		Sensor_StopAll();
		g_controls.state = STATE_STOPPED;

		pthread_mutex_unlock(&(g_controls.mutex));	
		return true;
	}
	pthread_mutex_unlock(&(g_controls.mutex));	
	return false;
}
