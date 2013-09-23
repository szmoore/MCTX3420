/**
 * @file bbb_pin.c
 * @purpose Implementation of BBB pin control functions and structures
 * THIS CODE IS NOT THREADSAFE
 */

#include "bbb_pin.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "options.h"

/**
 * Structure to represent a GPIO pin
 * Note: Only accessable to this file; to use the functions pass a GPIOId
 */
typedef struct
{
	int fd_value;
	int fd_direction;
} GPIO_Pin;

/**
 * Structure to represent an ADC pin
 * Note: Only accessable to this file; to use the functions pass a ADCId
 */
typedef struct
{
	int fd_value;
} ADC_Pin;

/**
 * Structure to represent a PWM pin
 * Note: Only accessable to this file; to use the functions pass a PWMId
 */
typedef struct
{
	int fd_run;
	FILE * file_duty;
	FILE * file_period;
	int fd_polarity;
} PWM_Pin;

/** Array of GPIO pins **/
static GPIO_Pin g_gpio[GPIO_NUM_PINS] = {{0}};
/** Array of ADC pins **/
static ADC_Pin g_adc[ADC_NUM_PINS] = {{0}};
/** Array of PWM pins **/
static PWM_Pin g_pwm[PWM_NUM_PINS] = {{0}};

static char g_buffer[BUFSIZ] = "";




/**
 * Export a GPIO pin and open the file descriptors
 */
void GPIO_Export(int pin)
{
	if (pin < 0 || pin > GPIO_NUM_PINS)
	{
		Abort("Invalid pin number %d", pin);
	}

	

	// Export the pin
	sprintf(g_buffer, "%s/export", GPIO_DEVICE_PATH);
	FILE * export = fopen(g_buffer, "w");
	if (export == NULL)
	{
		Abort("Couldn't open %s to export GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	fprintf(export, "%d", pin);	
	fclose(export);
	
	// Setup direction file descriptor
	sprintf(g_buffer, "%s/gpio%d/direction", GPIO_DEVICE_PATH, pin);
	g_gpio[pin].fd_direction = open(g_buffer, O_RDWR);
	if (g_gpio[pin].fd_direction < 0)
	{
		Abort("Couldn't open %s for GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}


	// Setup value file descriptor
	sprintf(g_buffer, "%s/gpio%d/value", GPIO_DEVICE_PATH, pin);
	g_gpio[pin].fd_value = open(g_buffer, O_RDWR);
	if (g_gpio[pin].fd_value < 0)
	{
		Abort("Couldn't open %s for GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	Log(LOGDEBUG, "Exported GPIO%d", pin);
	//sleep(1);
}

/**
 * Unexport a GPIO pin and close its' file descriptors
 */
void GPIO_Unexport(int pin)
{

	if (pin < 0 || pin > GPIO_NUM_PINS)
	{
		Abort("Invalid pin number %d", pin);
	}

	// Close file descriptors
	close(g_gpio[pin].fd_value);
	close(g_gpio[pin].fd_direction);

	// Unexport the pin

	if (g_buffer[0] == '\0')
		sprintf(g_buffer, "%s/unexport", GPIO_DEVICE_PATH);	
	FILE * export = fopen(g_buffer, "w");
	if (export == NULL)
	{
		Abort("Couldn't open %s to export GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	fprintf(export, "%d", pin);	
	fclose(export);
}




/**
 * Export all PWM pins and open file descriptors
 * @param pin - The pin number
 */
void PWM_Export(int pin)
{
	if (pin < 0 || pin > PWM_NUM_PINS)
	{
		Abort("Invalid pin number %d", pin);
	}
	
	// Export the pin
	sprintf(g_buffer, "%s/export", PWM_DEVICE_PATH);
	FILE * export = fopen(g_buffer, "w");
	if (export == NULL)
	{
		Abort("Couldn't open %s to export PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}
	
	fprintf(export, "%d\n", pin);
	fclose(export);

	// Open file descriptors
	sprintf(g_buffer, "%s/pwm%d/run", PWM_DEVICE_PATH, pin);
	g_pwm[pin].fd_run = open(g_buffer, O_WRONLY);
	if (g_pwm[pin].fd_run < 0)
	{
		Abort("Couldn't open %s for PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/polarity",PWM_DEVICE_PATH, pin);
	g_pwm[pin].fd_polarity = open(g_buffer, O_WRONLY);
	if (g_pwm[pin].fd_polarity < 0)
	{
		Abort("Couldn't open %s for PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/period_ns",PWM_DEVICE_PATH, pin);
	g_pwm[pin].file_period = fopen(g_buffer, "w");
	if (g_pwm[pin].file_period == NULL)
	{
		Abort("Couldn't open %s for PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/duty_ns",PWM_DEVICE_PATH, pin);
	g_pwm[pin].file_duty = fopen(g_buffer, "w");
	if (g_pwm[pin].file_duty == NULL)
	{
		Abort("Couldn't open %s for PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}

	// Don't buffer the streams
	setbuf(g_pwm[pin].file_period, NULL);
	setbuf(g_pwm[pin].file_duty, NULL);

	
}

/**
 * Unexport a PWM pin and close its file descriptors
 * @param pin - The pin number
 */
void PWM_Unexport(int pin)
{
	if (pin < 0 || pin > PWM_NUM_PINS)
	{
		Abort("Invalid pin number %d", pin);
	}

	// Close the file descriptors
	close(g_pwm[pin].fd_polarity);
	close(g_pwm[pin].fd_run);
	fclose(g_pwm[pin].file_period);
	fclose(g_pwm[pin].file_duty);

	//Unexport the pin
	sprintf(g_buffer, "%s/unexport", PWM_DEVICE_PATH);
	FILE * export = fopen(g_buffer, "w");
	if (export == NULL)
	{
		Abort("Couldn't open %s to unexport PWM pin %d - %s", g_buffer, pin, strerror(errno));	
	}
	
	fprintf(export, "%d", pin);
	fclose(export);


}

/**
 * Export ADC pins; http://beaglebone.cameon.net/home/reading-the-analog-inputs-adc
 * Can't use sysfs like GPIO or PWM pins
 * Bloody annoying how inconsistent stuff is on the Beaglebone
 */
void ADC_Export()
{
	for (int i = 0; i < ADC_NUM_PINS; ++i)
	{
		sprintf(g_buffer, "%s/AIN%d", g_options.adc_device_path, i);
		g_adc[i].fd_value = open(g_buffer, O_RDONLY);
		if (g_adc[i].fd_value < 0)
		{
			Abort("Couldn't open ADC %d device file %s - %s", i, g_buffer, strerror(errno));
		}

		//setbuf(g_adc[i].file_value, NULL);

	}
}

/**
 * Unexport ADC pins
 */
void ADC_Unexport()
{
	for (int i = 0; i < ADC_NUM_PINS; ++i)
		close(g_adc[i].fd_value);
}

/**
 * Set a GPIO pin
 * @param pin - The pin to set. MUST have been exported before calling this function.
 */
void GPIO_Set(int pin, bool value)
{
	if (pwrite(g_gpio[pin].fd_direction, "out", 3, 0) != 3)
	{
		Abort("Couldn't set GPIO %d direction - %s", pin, strerror(errno));
	}

	char c = '0' + (value);
	if (pwrite(g_gpio[pin].fd_value, &c, 1, 0) != 1)
	{
		Abort("Couldn't read GPIO %d value - %s", pin, strerror(errno));
	}

}

/** 
 * Read from a GPIO Pin
 * @param pin - The pin to read
 */
bool GPIO_Read(int pin)
{
	if (pwrite(g_gpio[pin].fd_direction, "in", 2, 0) != 2)
		Log(LOGERR,"Couldn't set GPIO %d direction - %s", pin, strerror(errno)); 
	char c = '0';
	if (pread(g_gpio[pin].fd_value, &c, 1, 0) != 1)
		Log(LOGERR,"Couldn't read GPIO %d value - %s", pin, strerror(errno));

	return (c == '1');

}

/**
 * Activate a PWM pin
 * @param pin - The pin to activate
 * @param polarity - if true, pin is active high, else active low
 * @param period - The period in ns
 * @param duty - The time the pin is active in ns
 */
void PWM_Set(int pin, bool polarity, long period, long duty)
{
	// Have to stop PWM before changing it
	if (pwrite(g_pwm[pin].fd_run, "0", 1, 0) != 1)
	{
		Abort("Couldn't stop PWM %d - %s", pin, strerror(errno));
	}

	char c = '0' + polarity;
	if (pwrite(g_pwm[pin].fd_polarity, &c, 1, 0) != 1)
	{
		Abort("Couldn't set PWM %d polarity - %s", pin, strerror(errno));
	}
	
	rewind(g_pwm[pin].file_period);	
	rewind(g_pwm[pin].file_duty);

	if (fprintf(g_pwm[pin].file_duty, "%lu", duty) == 0)
	{
		Abort("Couldn't set duty cycle for PWM %d - %s", pin, strerror(errno));
	}
	if (fprintf(g_pwm[pin].file_period, "%lu", period) == 0)
	{
		Abort("Couldn't set period for PWM %d - %s", pin, strerror(errno));
	}
	if (pwrite(g_pwm[pin].fd_run, "1", 1, 0) != 1)
	{
		Abort("Couldn't start PWM %d - %s", pin, strerror(errno));
	}

}

/**
 * Deactivate a PWM pin
 * @param pin - The pin to turn off
 */
void PWM_Stop(int pin)
{
	if (pwrite(g_pwm[pin].fd_run, "0", 1, 0) != 1)
	{
		Abort("Couldn't stop PWM %d - %s", pin, strerror(errno));
	}
}

/**
 * Read an ADC value
 * @param id - The ID of the ADC pin to read
 * @returns - The reading of the ADC channel
 */
int ADC_Read(int id)
{
	char adc_str[ADC_DIGITS] = "";
	lseek(g_adc[id].fd_value, 0, SEEK_SET);
	
	int i = 0;
	for (i = 0; i < ADC_DIGITS-1; ++i)
	{
		if (read(g_adc[id].fd_value, adc_str+i, 1) != 1)
			break;
		if (adc_str[i] == '\n')
		{
			adc_str[i] = '\0';
			break;
		}
	}

	char * end;
	int val = strtol(adc_str, &end, 10);
	if (*end != '\0')
	{
		Log(LOGERR, "Read non integer from ADC %d - %s", id, adc_str);
	}	
	return val;	
}
