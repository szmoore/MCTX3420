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
extern bool PWM_Stop(int pin);

#else
//Horrible hacks to silence gcc when compiling on systems that are not the BBB
extern bool True_Stub(int arg, ...);
extern bool ADC_Read_Stub(int *val, ...);
extern bool GPIO_Read_Stub(bool *val, ...);

#define GPIO_Export(pin) True_Stub((int)pin)
#define GPIO_Unexport(pin) (void)0

#define PWM_Export(pin) True_Stub((int)pin)
#define PWM_Unexport(pin) (void)0

#define ADC_Export(pin) True_Stub((int)pin)
#define ADC_Unexport(pin) (void)0

#define GPIO_Read(pin, result) GPIO_Read_Stub(result, pin)
#define GPIO_Set(pin, value) True_Stub((int)pin, value)

#define ADC_Read(id, value) ADC_Read_Stub(value, id)

#define PWM_Set(pin, polarity, period, duty) True_Stub((int)pin, polarity, period, duty)
#define PWM_Stop(pin) True_Stub((int)pin) 
//yuck

#endif //_BBB

#endif //_BBB_PIN_H

//EOF
