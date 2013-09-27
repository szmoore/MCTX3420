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
extern bool GPIO_Export(int pin);
extern void GPIO_Unexport(int pin);

extern bool PWM_Export(int pin);
extern void PWM_Unexport(int pin);

extern bool ADC_Export(int pin);
extern void ADC_Unexport(int pin);

// Pin reading/setting functions
extern bool GPIO_Read(int pin, bool *result);
extern bool GPIO_Set(int pin, bool value);

extern bool ADC_Read(int id, int *value);

extern bool PWM_Set(int pin, bool polarity, long period, long duty); // period and duty are in ns
extern void PWM_Stop(int pin);

#else
//Empty defines so it compiles on any platform that's not the BBB

extern bool GPIO_Export(int pin);
extern void GPIO_Unexport(int pin);

#define GPIO_Export(pin) true
#define GPIO_Unexport(pin)

#define PWM_Export(pin) true
#define PWM_Unexport(pin)

#define ADC_Export(pin) true
#define ADC_Unexport(pin)

#define GPIO_Read(pin, result) ((*(result) = 0) == 0)
#define GPIO_Set(pin, value) true

#define ADC_Read(id, value) ((*(value) = 0) == 0)

#define PWM_Set(pin, polarity, period, duty) true
#define PWM_Stop(pin)

#endif //_BBB

#endif //_BBB_PIN_H

//EOF
