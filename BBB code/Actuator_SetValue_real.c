#include "pwm.h"

char pin_dir = "/sys/class/gpio/gpio";		//move these
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

/**
 * Set an Actuator value
 * @param a - The Actuator
 * @param value - The value to set
 */
void Actuator_SetValue(Actuator * a, double value)
{
	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);

	DataPoint d = {TIMEVAL_DIFF(t, g_options.start_time), value};
	switch (a->id)
	{
		case ACTUATOR_TEST0:						//Pressure regulator example - requires PWM, 0-1000kPa input
		{	
			if (pwm_active == 0) {					//if inactive, start the pwm module
				pwm_init();
				pwm_start();
				pwm_set_period(FREQ);				//50Hz defined in pwm header file
			}
			if(value >= 0 && value <= 700) {
				double duty = value/1000 * 100; 	//convert pressure to duty percentage
				pwm_set_duty((int)duty);			//set duty percentage for actuator
			}
		} break;
		case ACTUATOR_TEST1:
		{
			SetPin(value, 1);						//Digital switch example - "1" is the pin number, to be specified by electronics team
		} break;
	}

	Log(LOGDEBUG, "Actuator %s set to %f", g_actuator_names[a->id], value);

	// Record the value
	Data_Save(&(a->data_file), &d, 1);
}