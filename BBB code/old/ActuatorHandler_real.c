#include "pwm.h"

char pin_dir = "/sys/class/gpio/gpio";
int pwm_active = 0;

/** Sets a GPIO pin to the desired value
*	@param value - the value (1 or 0) to write to the pin
*	@param pin_num - the number of the pin (refer to electronics team)
*/

void SetPin(int value, int pin_num) {
	int pin;
	char buffer[10];
	char pin_path[40];
	snprintf(pin_path, sizeof(pin_path), "%s%d%s", pin_dir, pin_num, "/value"); 	//create pin path
	pin = open(pin_path, O_WRONLY);
	if (pin < 0) perror("Failed to open pin");
	if (value) {
		strncpy(buffer, "1", ARRAY_SIZE(buffer) - 1);
	}
	else {
		strncpy(buffer, "0", ARRAY_SIZE(buffer) - 1);
	}
	int write = write(pin, buffer, strlen(buffer));
	if (write < 0) perror ("Failed to write to pin");
	close(pin);
}

//TODO: Be able to write to multiple PWM modules - easy to change
//		but current unsure about how many PWM signals we need

void ActuatorHandler(FCGIContext *context, ActuatorId id, const char *set_value) {
	char *ptr;
	
	switch(id) { 					//Add new actuators here
		case ACT_PRESSURE: 			//Suppose is pressure regulator. 0-700 input (kPa)
		{	
			if (pwm_active == 0) {	//if inactive, start the pwm module
				pwm_init();
				pwm_start();
				pwm_set_period(FREQ);				//50Hz defined in pwm header file
			}
			int value = strtol(set_value, &ptr, 10);
			//Beaglebone code
			if(value >= 0 && value <= 700) {
				double duty = value/700 * 100; 		//convert pressure to duty percentage
				pwm_set_duty((int)duty);			//set duty percentage for actuator
			}
			//server code
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
										//code to set pin to value
				SetPin(value, 1);		//"1" will need to be changed to pin numbers decided by electronics team
										//code for server
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