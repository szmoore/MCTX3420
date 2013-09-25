/**
 * @file pin_test.c
 * @purpose Implementations to allow direct control over pins through FastCGI
 */

#include "pin_test.h"

#include "bbb_pin.h"

/**
 * Export *ALL* pins for control
 */
void Pin_Init()
{
/*	for (int i = 0; i < GPIO_NUM_PINS; ++i)
		GPIO_Export(i);

	for (int i = 0; i < ADC_NUM_PINS; ++i)
		ADC_Export();
*/
	for (int i = 0; i < PWM_NUM_PINS; ++i)
		PWM_Export(i);
}

/**
 * Unexport all pins
 */
void Pin_Close()
{
	for (int i = 0; i < GPIO_NUM_PINS; ++i)
		GPIO_Unexport(i);

	for (int i = 0; i < ADC_NUM_PINS; ++i)
		ADC_Unexport(i);

	for (int i = 0; i < PWM_NUM_PINS; ++i)
		PWM_Unexport(i);
}

/**
 * Handle a request to the Pin test module
 * @param context - The FastCGI context
 * @param params - key/value pair parameters as a string
 */
void Pin_Handler(FCGIContext *context, char * params)
{
	
	char * type = NULL;
	int num = 0;
	bool set = true;
	bool pol = false;
	double freq = 50;
	double duty = 0.5;
	

	// key/value pairs
	FCGIValue values[] = {
		{"type", &type, FCGI_REQUIRED(FCGI_STRING_T)},
		{"num", &num, FCGI_REQUIRED(FCGI_INT_T)}, 
		{"set", &set, FCGI_BOOL_T},
		{"pol", &pol, FCGI_BOOL_T},
		{"freq", &freq, FCGI_DOUBLE_T},
		{"duty", &duty, FCGI_DOUBLE_T}
	};

	// enum to avoid the use of magic numbers
	typedef enum {
		TYPE,
		NUM,
		SET,
		POL,
		FREQ,
		DUTY
	} SensorParams;
	
	// Fill values appropriately
	if (!FCGI_ParseRequest(context, params, values, sizeof(values)/sizeof(FCGIValue)))
	{
		// Error occured; FCGI_RejectJSON already called
		return;
	}

	Log(LOGDEBUG, "Params: type = %s, num = %d, set = %d, pol = %d, freq = %f, duty = %f", type, num, set, pol, freq, duty);

	if (strcmp(type, "gpo") == 0)
	{
		if (num <= 0 || num > GPIO_NUM_PINS)
		{
			FCGI_RejectJSON(context, "Invalid GPIO pin");
			return;
		}

		Log(LOGDEBUG, "Setting GPIO%d to %d", num, set);
		GPIO_Set(num, set);

		FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
		FCGI_PrintRaw("GPIO%d set to %d\n", num, set);
	}
	else if (strcmp(type, "gpi") == 0)
	{
		if (num < 0 || num >= GPIO_NUM_PINS)
		{
			FCGI_RejectJSON(context, "Invalid GPIO pin");
			return;
		}
		Log(LOGDEBUG, "Reading GPIO%d", num);
		FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
		FCGI_PrintRaw("GPIO%d reads %d\n", num, GPIO_Read(num));

	}
	else if (strcmp(type, "adc") == 0)
	{
		if (num < 0 || num >= ADC_NUM_PINS)
		{
			FCGI_RejectJSON(context, "Invalid ADC pin");
			return;
		}
		Log(LOGDEBUG, "Reading ADC%d", num, set);
		FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
		FCGI_PrintRaw("ADC%d reads %d\n", num, ADC_Read(num));
	}
	else if (strcmp(type, "pwm") == 0)
	{
		if (num < 0 || num >= PWM_NUM_PINS)
		{
			FCGI_RejectJSON(context, "Invalid PWM pin");
			return;
		}

		FCGI_PrintRaw("Content-type: text/plain\r\n\r\n");
		
		if (set)
		{
			Log(LOGDEBUG, "Setting PWM%d", num);
			duty = duty < 0 ? 0 : duty > 1 ? 1 : duty;
			long period_ns = (long)(1e9 / freq);
			long duty_ns = (long)(duty * period_ns);
			PWM_Set(num, pol, period_ns, duty_ns);
			FCGI_PrintRaw("PWM%d set to period_ns = %lu (%f Hz), duty_ns = %lu (%f), polarity = %d", 
				num, period_ns, freq, duty_ns, duty*100, (int)pol);
		}
		else
		{
			Log(LOGDEBUG, "Stopping PWM%d",num);
			PWM_Stop(num);
			FCGI_PrintRaw("PWM%d stopped",num);
		}		
	}
	else
	{
		Log(LOGDEBUG, "Invalid pin type %s", type);
		FCGI_RejectJSON(context, "Invalid pin type");
	}

	

}

//EOF
