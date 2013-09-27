/**
 * @file bbb_pin.h
 * @brief Definition of functions for controlling pins on the Beaglebone Black
 */

#ifndef _BBB_PIN_H
#define _BBB_PIN_H

#include "common.h"

#include "bbb_pin_defines.h"

#if defined(_BBB) || defined(_BBB_PIN_SRC)
// Initialise / Deinitialise functions
extern void GPIO_Export(int pin);
extern void GPIO_Unexport(int pin);

extern void PWM_Export(int pin);
extern void PWM_Unexport(int pin);

extern void ADC_Export(int pin);
extern void ADC_Unexport(int pin);

// Pin reading/setting functions
extern bool GPIO_Read(int pin);
extern void GPIO_Set(int pin, bool value);

extern int ADC_Read(int pin);

extern void PWM_Set(int pin, bool polarity, long period, long duty); // period and duty are in ns
extern void PWM_Stop(int pin);
#else
//Empty defines so it compiles on any platform that's not the BBB
#define GPIO_Export(pin)
#define GPIO_Unexport(pin)

#define PWM_Export(pin)
#define PWM_Unexport(pin)

#define ADC_Export(pin)
#define ADC_Unexport(pin)

#define GPIO_Read(pin) 0
#define GPIO_Set(pin, value)

#define ADC_Read(pin) 0

#define PWM_Set(pin, polarity, period, duty)
#define PWM_Stop(Pin)


#endif //_BBB

#endif //_BBB_PIN_H

//EOF
