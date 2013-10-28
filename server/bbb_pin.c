/**
 * @file bbb_pin.c
 * @brief Implementation of BBB pin control functions and structures
 * On non-beaglebone (actually non-arm) platforms, this code is disabled.
 * THIS CODE IS NOT THREADSAFE
 */

#define _BBB_PIN_SRC
#include "bbb_pin.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "options.h"

/**
 * Structure to represent a GPIO pin
 * Note: Only accessable to this file; to use the functions pass a GPIOId
 */
typedef struct
{
	bool initialised;
	int fd_value;
	int fd_direction;
} GPIO_Pin;

/**
 * Structure to represent an ADC pin
 * Note: Only accessable to this file; to use the functions pass a ADCId
 */
typedef struct
{
	bool initialised;
	int fd_value;
} ADC_Pin;

/**
 * Structure to represent a PWM pin
 * Note: Only accessable to this file; to use the functions pass a PWMId
 */
typedef struct
{
	bool initialised;
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

static char g_buffer[BUFSIZ] = {0};

/**
 * Maps a GPIO number to an index into g_gpio (only for use in bbb_pin.c)
 * If there is no index for that GPIO number, 128 is returned.
 */
const unsigned char g_pin_gpio_to_index[GPIO_MAX_NUMBER+1] = {
	128, 128, 128, 128,   0,   1, 128, 128,   2,   3,   4,   5, 128, 128,
	  6,   7, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,   8,   9,
	128, 128,  10,  11, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128,  12,  13,  14,  15,  16,  17, 128, 128, 128, 128, 128, 128,
	128, 128, 128, 128,  18,  19, 128, 128, 128,  20,  21,  22,  23,  24,
	 25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36, 128, 128,
	128, 128,  37,  38,  39,  40, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
	 41, 128, 128,  42
};

/**
 * Maps an index in g_gpio to the corresponding GPIO number.
 */
const unsigned char g_pin_index_to_gpio[GPIO_NUM_PINS] = {
	  4,   5,   8,   9,  10,  11,  14,  15,  26,  27,  30,  31,  44,  45,
	 46,  47,  48,  49,  60,  61,  65,  66,  67,  68,  69,  70,  71,  72,
	 73,  74,  75,  76,  77,  78,  79,  80,  81,  86,  87,  88,  89, 112,
	115
};

/**
 * Export a GPIO pin and open the file descriptors
 * @param pin The GPIO number to be exported
 * @return true on success, false otherwise
 */
bool GPIO_Export(int pin)
{
	if (pin < 0 || pin > GPIO_MAX_NUMBER || g_pin_gpio_to_index[pin] == 128)
	{
		AbortBool("Not a useable pin number: %d", pin);
	}

	GPIO_Pin *gpio = &g_gpio[g_pin_gpio_to_index[pin]];
	if (gpio->initialised)
	{
		Log(LOGNOTE, "GPIO %d already initialised.", pin);
		return true;
	}

	// Export the pin
	sprintf(g_buffer, "%s/export", GPIO_DEVICE_PATH);
	FILE * file_export = fopen(g_buffer, "w");
	if (file_export == NULL)
	{
		AbortBool("Couldn't open %s to export GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}
	fprintf(file_export, "%d", pin);	
	fclose(file_export);
	
	// Setup direction file descriptor
	sprintf(g_buffer, "%s/gpio%d/direction", GPIO_DEVICE_PATH, pin);
	gpio->fd_direction = open(g_buffer, O_RDWR);
	if (gpio->fd_direction < 0)
	{
		AbortBool("Couldn't open %s for GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	// Setup value file descriptor
	sprintf(g_buffer, "%s/gpio%d/value", GPIO_DEVICE_PATH, pin);
	gpio->fd_value = open(g_buffer, O_RDWR);
	if (gpio->fd_value < 0)
	{
		close(gpio->fd_direction);
		AbortBool("Couldn't open %s for GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	gpio->initialised = true;
	Log(LOGDEBUG, "Exported GPIO%d", pin);
	return true;
}

/**
 * Unexport a GPIO pin and close its' file descriptors
 * @param pin The GPIO number to be unexported
 */
void GPIO_Unexport(int pin)
{
	if (pin < 0 || pin > GPIO_MAX_NUMBER || g_pin_gpio_to_index[pin] == 128)
	{
		Abort("Not a useable pin number: %d", pin);
	}

	GPIO_Pin *gpio = &g_gpio[g_pin_gpio_to_index[pin]];
	if (!gpio->initialised)
	{
		Abort("GPIO %d is already uninitialised", pin);
	}

	// Close file descriptors
	close(gpio->fd_value);
	close(gpio->fd_direction);
	// Uninitialise this one
	gpio->initialised = false;

	// Unexport the pin
	if (g_buffer[0] == '\0')
		sprintf(g_buffer, "%s/unexport", GPIO_DEVICE_PATH);	
	FILE * file_export = fopen(g_buffer, "w");
	if (file_export == NULL)
	{
		Abort("Couldn't open %s to export GPIO pin %d - %s", g_buffer, pin, strerror(errno));
	}

	fprintf(file_export, "%d", pin);	
	fclose(file_export);
}

/**
 * Initialise all PWM pins and open file descriptors
 * @param pin - The sysfs pin number
 * @return true if exported, false otherwise
 */
bool PWM_Export(int pin)
{
	//goto would make this easier...
	if (pin < 0 || pin >= PWM_NUM_PINS)
	{
		AbortBool("Invalid PWM pin %d specified.", pin);
	}

	PWM_Pin *pwm = &g_pwm[pin];
	if (pwm->initialised)
	{
		Log(LOGNOTE, "PWM %d already exported.", pin);
		return true;
	}

	// Try export the pin, doesn't matter if it's already exported.
	sprintf(g_buffer, "%s/export", PWM_DEVICE_PATH);
	FILE * file_export = fopen(g_buffer, "w");
	if (file_export == NULL)
	{
		AbortBool("Couldn't open %s to export PWM pin %d - %s", 
				g_buffer, pin, strerror(errno));
	}
	fprintf(file_export, "%d\n", pin);
	fclose(file_export);

	// Open file descriptors
	sprintf(g_buffer, "%s/pwm%d/run", PWM_DEVICE_PATH, pin);
	pwm->fd_run = open(g_buffer, O_WRONLY);
	if (pwm->fd_run < 0)
	{
		AbortBool("Couldn't open %s for PWM%d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/polarity", PWM_DEVICE_PATH, pin);
	pwm->fd_polarity = open(g_buffer, O_WRONLY);
	if (pwm->fd_polarity < 0)
	{
		close(pwm->fd_run);
		AbortBool("Couldn't open %s for PWM%d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/period_ns", PWM_DEVICE_PATH, pin);
	pwm->file_period = fopen(g_buffer, "w");
	if (pwm->file_period == NULL)
	{
		close(pwm->fd_run);
		close(pwm->fd_polarity);
		AbortBool("Couldn't open %s for PWM%d - %s", g_buffer, pin, strerror(errno));
	}

	sprintf(g_buffer, "%s/pwm%d/duty_ns", PWM_DEVICE_PATH, pin);
	pwm->file_duty = fopen(g_buffer, "w");
	if (pwm->file_duty == NULL)
	{
		close(pwm->fd_run);
		close(pwm->fd_polarity);
		fclose(pwm->file_period);
		AbortBool("Couldn't open %s for PWM%d - %s", g_buffer, pin, strerror(errno));
	}

	// Don't buffer the streams
	setbuf(pwm->file_period, NULL);
	setbuf(pwm->file_duty, NULL);	

	pwm->initialised = true;
	Log(LOGDEBUG, "Exported PWM%d", pin);
	return true;
}


/**
 * Unexport a PWM pin and close its file descriptors
 * @param pin - The sysfs pin number
 */
void PWM_Unexport(int pin)
{
	if (pin < 0 || pin >= PWM_NUM_PINS)
	{
		Abort("Invalid PWM pin number %d specified.", pin);
	}

	PWM_Pin *pwm = &g_pwm[pin];
	if (!pwm->initialised)
	{
		Abort("PWM %d not initialised", pin);
	}

	// Close the file descriptors
	close(pwm->fd_polarity);
	//Stop it, if it's still running
	pwrite(pwm->fd_run, "0", 1, 0);
	close(pwm->fd_run);
	fclose(pwm->file_period);
	fclose(pwm->file_duty);

	pwm->initialised = false;

	// Try unexport the pin, doesn't matter if it's already unexported.
	sprintf(g_buffer, "%s/unexport", PWM_DEVICE_PATH);
	FILE * file_unexport = fopen(g_buffer, "w");
	if (file_unexport == NULL)
	{
		Abort("Couldn't open %s to unexport PWM pin %d - %s", g_buffer, pin, strerror(errno));
	}
	fprintf(file_unexport, "%d\n", pin);
	fclose(file_unexport);
}

/**
 * Initialise ADC structures
 * @param pin The ADC pin number
 */
bool ADC_Export(int pin)
{
	if (pin < 0 || pin >= ADC_NUM_PINS)
	{
		AbortBool("Invalid ADC pin %d specified.", pin);
	}
	else if (g_adc[pin].initialised)
	{
		Log(LOGNOTE, "ADC %d already initialised", pin);
		return true;
	}

	sprintf(g_buffer, "%s/in_voltage%d_raw", ADC_DEVICE_PATH, pin);
	g_adc[pin].fd_value = open(g_buffer, O_RDONLY);
	if (g_adc[pin].fd_value <0)
	{
		AbortBool("Couldn't open ADC %d device file %s - %s", pin, g_buffer, strerror(errno));
	}

	g_adc[pin].initialised = true;
	Log(LOGDEBUG, "Opened ADC %d", pin);
	return true;
}

/**
 * Unexport ADC pins
 * @param pin The ADC pin number
 */
void ADC_Unexport(int pin)
{
	if (pin < 0 || pin >= ADC_NUM_PINS)
	{
		Abort("Invalid ADC pin %d specified.", pin);
	}
	else if (!g_adc[pin].initialised)
	{
		Abort("ADC %d already uninitialised", pin);
	}

	close(g_adc[pin].fd_value);	
	g_adc[pin].fd_value = -1;
	g_adc[pin].initialised = false;
}

/**
 * Set a GPIO pin
 * @param pin - The pin to set. MUST have been exported before calling this function.
 */
bool GPIO_Set(int pin, bool value)
{
	if (pin < 0 || pin > GPIO_MAX_NUMBER || g_pin_gpio_to_index[pin] == 128)
	{
		AbortBool("Not a useable pin number: %d", pin);
	}

	GPIO_Pin *gpio = &g_gpio[g_pin_gpio_to_index[pin]];
	if (!gpio->initialised)
	{
		AbortBool("GPIO %d is not initialised.", pin);
	}
	//Set the pin direction
	if (pwrite(gpio->fd_direction, "out", 3, 0) != 3)
	{
		AbortBool("Couldn't set GPIO %d direction - %s", pin, strerror(errno));
	}

	char c = value ? '1' : '0';
	if (pwrite(gpio->fd_value, &c, 1, 0) != 1)
	{
		AbortBool("Couldn't read GPIO %d value - %s", pin, strerror(errno));
	}

	return true;
}

/** 
 * Read from a GPIO Pin
 * @param pin - The pin to read
 * @param result A pointer to store the result
 * @return true on success, false otherwise
 */
bool GPIO_Read(int pin, bool *result)
{
	if (pin < 0 || pin > GPIO_MAX_NUMBER || g_pin_gpio_to_index[pin] == 128)
	{
		AbortBool("Not a useable pin number: %d", pin);
	}

	GPIO_Pin *gpio = &g_gpio[g_pin_gpio_to_index[pin]];
	if (!gpio->initialised)
	{
		AbortBool("GPIO %d is not initialised.", pin);
	}

	if (pwrite(gpio->fd_direction, "in", 2, 0) != 2)
	{
		AbortBool("Couldn't set GPIO %d direction - %s", pin, strerror(errno));
	}
	
	char c = '0';
	if (pread(gpio->fd_value, &c, 1, 0) != 1)
	{
		AbortBool("Couldn't read GPIO %d value - %s", pin, strerror(errno));
	}

	*result = (c == '1');
	return true;
}

/**
 * Activate a PWM pin
 * @param pin - The sysfs pin number
 * @param polarity - if true, pin is active high, else active low
 * @param period - The period in ns
 * @param duty - The time the pin is active in ns
 */
bool PWM_Set(int pin, bool polarity, long period, long duty)
{
	Log(LOGDEBUG, "Pin %d, pol %d, period: %lu, duty: %lu", pin, polarity, period, duty);
	
	if (pin < 0 || pin >= PWM_NUM_PINS)
	{
		AbortBool("Invalid PWM pin number %d specified.", pin);
	}

	PWM_Pin *pwm = &g_pwm[pin];
	if (!pwm->initialised)
	{
		AbortBool("PWM %d is not initialised.", pin);
	}

	// Have to stop PWM before changing it
	if (pwrite(pwm->fd_run, "0", 1, 0) != 1)
	{
		AbortBool("Couldn't stop PWM %d - %s", pin, strerror(errno));
	}

	char c = polarity ? '1' : '0';
	if (pwrite(pwm->fd_polarity, &c, 1, 0) != 1)
	{
		AbortBool("Couldn't set PWM %d polarity - %s", pin, strerror(errno));
	}

	//This must be done first, otherwise period/duty settings can conflict
	if (fwrite("0", 1, 1, pwm->file_duty) < 1)
	{
		AbortBool("Couldn't zero the duty for PWM %d - %s", pin, strerror(errno));
	}

	if (fprintf(pwm->file_period, "%lu", period) < 0)
	{
		AbortBool("Couldn't set period for PWM %d - %s", pin, strerror(errno));
	}


	if (fprintf(pwm->file_duty, "%lu", duty) < 0)
	{
		AbortBool("Couldn't set duty cycle for PWM %d - %s", pin, strerror(errno));
	}


	if (pwrite(pwm->fd_run, "1", 1, 0) != 1)
	{
		AbortBool("Couldn't start PWM %d - %s", pin, strerror(errno));
	}

	return true;
}

/**
 * Deactivate a PWM pin
 * @param pin - The syfs pin number
 * @return true on success, false otherwise
 */
bool PWM_Stop(int pin)
{
	if (pin < 0 || pin >= PWM_NUM_PINS)
	{
		AbortBool("Invalid PWM pin number %d specified.", pin);
	}
	else if (!g_pwm[pin].initialised)
	{
		AbortBool("PWM %d is not initialised.", pin);
	}

	if (pwrite(g_pwm[pin].fd_run, "0", 1, 0) != 1)
	{
		AbortBool("Couldn't stop PWM %d - %s", pin, strerror(errno));
	}

	return true;
}

/**
 * Read an ADC value
 * @param id - The ID of the ADC pin to read
 * @param value - A pointer to store the value read from the ADC
 * @returns - The true if succeeded, false otherwise.
 */
bool ADC_Read(int id, int *value)
{
	char adc_str[ADC_DIGITS] = {0};

	if (id < 0 || id >= ADC_NUM_PINS)
	{
		AbortBool("Invalid ADC pin %d specified.", id);
	}
	else if (!g_adc[id].initialised)
	{
		AbortBool("ADC %d is not initialised.", id);
	}

	if (pread(g_adc[id].fd_value, adc_str, ADC_DIGITS-1, 0) == -1)
	{
		//AbortBool("ADC %d read failed: %s", id, strerror(errno));
		return false;
	}

	*value = strtol(adc_str, NULL, 10);
	return true;
}

#ifndef _BBB
//For running on systems that are not the BBB
bool True_Stub(int arg, ...) { return true; }
bool ADC_Read_Stub(int *val, ...) { *val = 0; return true; }
bool GPIO_Read_Stub(bool *val, ...) { *val = false; return true; }
#endif
