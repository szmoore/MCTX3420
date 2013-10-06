/**
 * @file bbb_pin_defines.h
 * @brief Defines pins on Beaglebone Black
 */

#ifndef _BBB_PIN_DEFINES_H
#define _BBB_PIN_DEFINES_H

/** The number of expansion pins on the BBB **/
#define BBB_PIN_COUNT 92

/** GPIO0 defines **/

#define GPIO0_1 1
#define GPIO0_2 2 // Used for PWM
#define GPIO0_3 3 // Used for PWM
#define GPIO0_4 4 
#define GPIO0_5 5
#define GPIO0_6 6
#define GPIO0_7 7
#define GPIO0_8 8
#define GPIO0_9 9
#define GPIO0_10 10
#define GPIO0_11 11
#define GPIO0_12 12
#define GPIO0_13 13
#define GPIO0_14 14
#define GPIO0_15 15
#define GPIO0_16 16
#define GPIO0_17 17
#define GPIO0_18 18
#define GPIO0_19 19
#define GPIO0_20 20
#define GPIO0_21 21
#define GPIO0_22 22
#define GPIO0_23 23
#define GPIO0_24 24
#define GPIO0_25 25
#define GPIO0_26 26
#define GPIO0_27 27
#define GPIO0_28 28
#define GPIO0_29 29
#define GPIO0_30 30
#define GPIO0_31 31
#define GPIO0_32 32

/** GPIO1 defines **/

#define GPIO1_1 33
#define GPIO1_2 34
#define GPIO1_3 35
#define GPIO1_4 36
#define GPIO1_5 37
#define GPIO1_6 38
#define GPIO1_7 39
#define GPIO1_8 40
#define GPIO1_9 41
#define GPIO1_10 42
#define GPIO1_11 43
#define GPIO1_12 44
#define GPIO1_13 45
#define GPIO1_14 46
#define GPIO1_15 47
#define GPIO1_16 48
#define GPIO1_17 49
#define GPIO1_18 50
#define GPIO1_19 51
#define GPIO1_20 52
#define GPIO1_21 53
#define GPIO1_22 54
#define GPIO1_23 55
#define GPIO1_24 56
#define GPIO1_25 57
#define GPIO1_26 58
#define GPIO1_27 59
#define GPIO1_28 60
#define GPIO1_29 61
#define GPIO1_30 62
#define GPIO1_31 63
#define GPIO1_32 64

/** GPIO2 defines **/

#define GPIO2_1 65
#define GPIO2_2 66
#define GPIO2_3 67
#define GPIO2_4 68
#define GPIO2_5 69
#define GPIO2_6 70
#define GPIO2_7 71
#define GPIO2_8 72
#define GPIO2_9 73
#define GPIO2_10 74
#define GPIO2_11 75
#define GPIO2_12 76
#define GPIO2_13 77
#define GPIO2_14 78
#define GPIO2_15 79
#define GPIO2_16 80
#define GPIO2_17 81
#define GPIO2_18 82
#define GPIO2_19 83
#define GPIO2_20 84
#define GPIO2_21 85
#define GPIO2_22 86
#define GPIO2_23 87
#define GPIO2_24 88
#define GPIO2_25 89
#define GPIO2_26 90
#define GPIO2_27 91
#define GPIO2_28 92
#define GPIO2_29 93
#define GPIO2_30 94
#define GPIO2_31 95
#define GPIO2_32 96

/** Export path **/
#define GPIO_DEVICE_PATH "/sys/class/gpio"

/** Number of useable GPIO pins **/
#define GPIO_NUM_PINS 43
/** The max usable GPIO number **/
#define GPIO_MAX_NUMBER 115

/* Luts */
extern const unsigned char g_pin_gpio_to_index[GPIO_MAX_NUMBER+1];
extern const unsigned char g_pin_index_to_gpio[GPIO_NUM_PINS];

#define ADC_BITS 12
#define ADC_DIGITS 5
#define ADC0 0
#define ADC1 1
#define ADC2 2
#define ADC3 3
#define ADC4 4
#define ADC5 5
#define ADC6 6
#define ADC7 7

/** Number of ADC pins **/
#define ADC_NUM_PINS 8

#define ADC_DEVICE_PATH "/sys/bus/iio/devices/iio:device0/"

/** PWM names to sysfs numbers **/
#define EHRPWM0A 0 //P9_22
#define EHRPWM0B 1 //P9_21 - period paired with EHRPWM0A
#define EHRPWM1A 3 //P9_14
#define EHRPWM1B 4 //P9_16 - period paired with EHRPWM1A
#define ECAP0    2 //P9_42
#define ECAP2	 7 //P9_28
#define EHRPWM2A 5 //P8_19
#define EHRPWM2B 6 //P8_13 - period paired with EHRPWM2A

/** Number of PWM pins **/
#define PWM_NUM_PINS 8

/** Path to PWM sysfs **/
#define PWM_DEVICE_PATH "/sys/class/pwm"

#endif //_BBB_PIN_DEFINES_H


