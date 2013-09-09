void ActuatorHandler(FCGIContext *context, ActuatorId id, const char *set_value) {
	char *ptr;
	
	switch(id) { //Add new actuators here
		case ACT_PRESSURE: //Suppose is pressure regulator. 0-700 input (kPa)
		//requires PWM control
		//TBA, currently in a separate file
		//pwm_convert_duty(value); - convert input to required duty cycle
		//pwm_set_frequency(PWM_id, freq); - can be done during PWM setup, frequency won't change (50Hz?)
		//pwm_set_duty_cycle(PWM_id, duty_cycle); - this is the low/high percentage, will vary with input
		//may also need to set polarity?
		//if pwm is not setup, run setup functions first
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
		//requires digital control
		{
			int value = strtol(set_value, &ptr, 10);
			if (*ptr == '\0') {
				//code to set pin to value
				//move this to separate function, will need to be repeated
				int fd;
				char buffer[10];
				if ((fd = open("/sys/class/gpio/gpio50/value", O_WRONLY)) < 0)	//randomly chose pin 50 (each pin can be mapped to a certain switch case as required)
					perror("Failed to open pin");
				if (value) {
					strncpy(buffer, "1", ARRAY_SIZE(buffer) - 1);				//copy value to buffer
				} else {
					strncpy(buffer, "0", ARRAY_SIZE(buffer) - 1);
				}
				int write = write(fd, buffer, strlen(buffer));					//write buffer to pin
				if (write < 0) perror("Failed to write to pin");
				close(fd);
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