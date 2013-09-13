/**
 * @file control.c
 * @brief Handles all client control requests (admin/actuator related)
 */
#include "common.h"
#include "control.h"

const char * g_actuator_names[NUMACTUATORS] = {	
	"Pressure regulator", "Solenoid 1" 
};

/**
 * Handles control of the actuators.
 */
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
}

/**
 * System control handler. This covers control over all aspects of the system.
 * E.g: Actuators, system commands (start/stop experiment/recording) etc
 * @param context The context to work in
 * @param params The input parameters
 */
void Control_Handler(FCGIContext *context, char *params) {
	const char *action, *key = "", *mode = "", *name = "";
	bool force = false;

	FCGIValue values[5] = {
		{"action", &action, FCGI_REQUIRED(FCGI_STRING_T)},
		{"key", &key, FCGI_STRING_T},
		{"force", &force, FCGI_BOOL_T},
		{"mode", &mode, FCGI_STRING_T},
		{"name", &name, FCGI_STRING_T}
	};

	if (!FCGI_ParseRequest(context, params, values, 5))
		return;

	if (!strcmp(action, "gain")) {
		FCGI_BeginControl(context, force);
	} else { 
		if (!FCGI_HasControl(context, key)) {
			FCGI_RejectJSONEx(context, 
				STATUS_UNAUTHORIZED, "Invalid control key specified.");
			
		} else if (!strcmp(action, "release")) {
			FCGI_EndControl(context);
		} else if (!strcmp(action, "experiment")) {
			if (!strcmp(mode, "start")) {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_JSONPair("description", mode);
				FCGI_EndJSON();
			} else if (!strcmp(mode, "pause")) {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_JSONPair("description", mode);
				FCGI_EndJSON();
			} else if (!strcmp(mode, "stop")) {
				FCGI_BeginJSON(context, STATUS_OK);
				FCGI_JSONPair("description", mode);
				FCGI_EndJSON();
			} else {
				FCGI_RejectJSON(context, "Unknown experiment mode specified");
			}
		} else {
			FCGI_RejectJSON(context, "Unknown action specified");
		}
	}
}
