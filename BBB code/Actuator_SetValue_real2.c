#include "gpio.h"
#include "pwm.h"

void Actuator_SetValue(Actuator * a, double value)
{
	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);

	DataPoint d = {TIMEVAL_DIFF(t, g_options.start_time), value};
	switch (a->id)
	{
		case ACTUATOR_TEST0:			//LED actuator test code, should blink onboard LED next to Ethernet port
			FILE *LEDHandle = NULL;		//code reference: http://learnbuildshare.wordpress.com/2013/05/19/beaglebone-black-controlling-user-leds-using-c/
			char *LEDBrightness = "/sys/class/leds/beaglebone\:green\:usr0/brightness";
			if(value == 1) {
				if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL) {
					fwrite("1", sizeof(char), 1, LEDHandle);
					fclose(LEDHandle);
				}
			else if(value == 0) {
				if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL) {
					fwrite("0", sizeof(char), 1, LEDHandle);
					fclose(LEDHandle);
			}
			else perror("Pin value should be 1 or 0");
			break;
		case ACTUATOR_TEST1:
			// Quick actuator function for testing pins
			// GPIOPin can be either passed as an argument, or defined here (as pins won't change)
			// First way is better and more generalised
			int GPIOPin = 13;
			// Modify this to only export on first run, unexport on shutdown
			pinExport(setValue, GPIOString);
			pinDirection(GPIODirection, setValue);
			pinSet(value, GPIOValue, setValue);
			pinUnexport(setValue, GPIOString);
			break;
		case ACTUATOR_TEST2:
			if (pwminit == 0) {					//if inactive, start the pwm module
				pwm_init();
			}
			if (pwmstart == 0) {
				pwm_start();
				pwm_set_period(FREQ);				//50Hz defined in pwm header file
			}
			if(value >= 0 && value <= 1000) {
				double duty = value/1000 * 100; 	//convert pressure to duty percentage
				pwm_set_duty((int)duty);			//set duty percentage for actuator
			}
	}
	Log(LOGDEBUG, "Actuator %s set to %f", g_actuator_names[a->id], value);
	// Record the value
	Data_Save(&(a->data_file), &d, 1);
}