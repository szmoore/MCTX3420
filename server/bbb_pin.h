/**
 * @file bbb_pin.h
 * @brief Definition of functions for controlling pins on the Beaglebone Black
 */

#ifndef _BBB_PIN_H
#define _BBB_PIN_H

#include "common.h"

#include "bbb_pin_defines.h"

// Initialise / Deinitialise functions
extern void GPIO_Export(int pin);
extern void GPIO_Unexport(int pin);

extern void PWM_Export(int pin);
extern void PWM_Unexport(int pin);

extern void ADC_Export();
extern void ADC_Unexport();

// Pin reading/setting functions
extern bool GPIO_Read(int pin);
extern void GPIO_Set(int pin, bool value);

extern int ADC_Read(int pin);

extern void PWM_Set(int pin, bool polarity, long period, long duty); // period and duty are in ns
extern void PWM_Stop(int pin);



#endif //_BBB_PIN_H

//EOF
