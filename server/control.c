/**
 * @file control.c
 * @brief Handles all client control requests (admin/actuator related)
 */
#include "common.h"
#include "control.h"

/**
 * Handles control of the actuators.
 */
void ActuatorHandler(FCGIContext *context, int id, const char *set_value) {
	char *ptr;
	
	switch(id) { //Add new actuators here
		case ACT_PREG: //Suppose is pressure regulator. 0-700 input (kPa)
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
				FCGI_RejectJSON(context);
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
	const char *key, *value, *control_key = NULL;
	const char *action = NULL, *set_value = NULL;
	bool force = false;
	char *ptr;
	int id = ACT_NONE;
	
	while ((params = FCGI_KeyPair(params, &key, &value))) {
		if (!strcmp(key, "action"))
			action = value;
		else if (!strcmp(key, "key"))
			control_key = value;
		else if (!strcmp(key, "force"))
			force = !force;
		else if (!strcmp(key, "id") && *value) { //Ensure non-empty value
			int parsed = strtol(value, &ptr, 10);
			if (*ptr == '\0') {
				id = parsed;
			}
		} else if (!strcmp(key, "value")) {
			set_value = value;
		}
	}
	
	if (action == NULL) { //Must have an action
		FCGI_RejectJSON(context);
	} else if (!strcmp(action, "start")) {
		FCGI_BeginControl(context, force);
	} else if (!strcmp(action, "stop")) { //Don't require control key to stop...
		//EMERGENCY STOP!! TODO - replace!
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("description", "stopped! (not)");
		FCGI_EndJSON();
	} else { //Under this section, the user must have the current control key.
		if (!FCGI_HasControl(context, control_key)) {
			FCGI_RejectJSONEx(context, 
				STATUS_UNAUTHORIZED, "Invalid control key specified.");
		} else if (!strcmp(action, "end")) {
			FCGI_EndControl(context);
		} else if (!strcmp(action, "set")) {
			if (set_value == NULL || *set_value == '\0') {
				FCGI_RejectJSONEx(context, 
					STATUS_ERROR, "Set called but no value specified.");
			} else {
				ActuatorHandler(context, id, set_value);
			}
		}
	}
}
