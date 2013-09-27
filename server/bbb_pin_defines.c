#include "bbb_pin_defines.h"

/* Luts and stuff. Yay magic numbers **/

/** 
 * A lookup table from the actual pin number to GPIO number.
 * e.g P8_13 is g_pin_real_to_gpio[0*46+13] = g_pin_real_to_gpio[13]
 * e.g P9_13 is g_pin_real_to_gpio[1*46+13] = g_pin_real_to_gpio[59]
 *
 * Where the returned value is 0, there is no GPIO pin
 * at that location.
 */

const unsigned char g_pin_real_to_gpio[BBB_PIN_COUNT+1] = {
	  0,   0,   0,   0,   0,   0,   0,  66,  67,  69,  68,  45,  44,   0,
	 26,  47,  46,  27,  65,   0,   0,   0,   0,   0,   0,   0,  61,  86,
	 88,  87,  89,  10,  11,   9,  81,   8,  80,  78,  79,  76,  77,  74,
	 75,  72,  73,  70,  71,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,  30,  60,  31,   0,  48,   0,   5,   4,   0,   0,   0,   0,  49,
	 15,   0,  14, 115,   0,   0, 112,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0
};

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
	 41, 128, 128,  42, 128, 128
};

/**
 * Maps an index in g_gpio to the corresponding GPIO number.
 */
const unsigned char g_gpio_lut[GPIO_NUM_PINS] = {
	  4,   5,   8,   9,  10,  11,  14,  15,  26,  27,  30,  31,  44,  45,
	 46,  47,  48,  49,  60,  61,  65,  66,  67,  68,  69,  70,  71,  72,
	 73,  74,  75,  76,  77,  78,  79,  80,  81,  86,  87,  88,  89, 112,
	115
};

/**
 * Converts PWM index to PWM number
 * e.g index 3 becomes 6 for /sys/class/pwm/pwm6
 */
const unsigned char g_pin_safe_pwm[PWM_NUM_SAFE_PINS] = {
	0, 2, 4, 6, 7
}; //blergh