#include "common.h"
#include "control.h"

/**
 * System control handler. This covers control over all aspects of the system.
 * E.g: Actuators, system commands (start/stop experiment/recording) etc
 * @param context The context to work in
 * @param params The input parameters
 */
void Control_Handler(FCGIContext *context, char *params) {
	const char *key, *value, *loginkey = NULL, *action = NULL;
	bool force = false;
	
	while ((params = FCGI_KeyPair(params, &key, &value))) {
		if (!strcmp(key, "action"))
			action = value;
		else if (!strcmp(key, "key"))
			loginkey = value;
		else if (!strcmp(key, "force"))
			force = !force;
		else if (!strcmp(key, "id")) {
		
		}
		else if (!strcmp(key, "value")) {
		
		}
	}
	
	if (!strcmp(action, "start")) {
		FCGI_Authorize(context, force);
	} else if (!strcmp(action, "stop")) { //Don't require control key to stop...
		//EMERGENCY STOP!!
		FCGI_BeginJSON(context, STATUS_OK);
		FCGI_JSONPair("description", "stopped!"); //Not really
		FCGI_EndJSON();
	} else {
		if (!FCGI_Authorized(context, loginkey)) {
			FCGI_BeginJSON(context, STATUS_UNAUTHORIZED);
			FCGI_JSONPair("description", "Invalid key specified.");
			FCGI_EndJSON();
			return;
		} else if (!strcmp(action, "end")) {
			FCGI_AuthorizeEnd(context);
		} else if (!strcmp(action, "set")) {
			FCGI_BeginJSON(context, STATUS_OK);
			FCGI_JSONPair("description", "actuated!");
			FCGI_EndJSON();
		}
	}
}
