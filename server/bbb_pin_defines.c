#include "bbb_pin_defines.h"

/** 
 * A lookup table from header number to GPIO pin number.
 * e.g P8_13 is g_gpio_lut[0*46+13] = g_gpio_lut[13]
 * e.g P9_13 is g_gpio_lut[1*46+13] = g_gpio_lut[59]
 *
 * Where the returned value is 0, there is no GPIO pin
 * at that location.
 */
const unsigned char g_pin_to_gpio[GPIO_LUT_SIZE] = {
	  0,   0,   0,   0,   0,   0,   0,  66,  67,  69,  68,  45,  44,  23,
	 26,  47,  46,  27,  65,  22,   0,   0,   0,   0,   0,   0,  61,  86,
	 88,  87,  89,  10,  11,   9,  81,   8,  80,  78,  79,  76,  77,  74,
	 75,  72,  73,  70,  71,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,  30,  60,  31,  50,  48,  51,   5,   4,   0,   0,   3,   2,  49,
	 15, 117,  14, 115,   0,   0, 112,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0
};

/**
 * Converts GPIO number to index for g_gpio, or 128 if no map.
 */
const unsigned char g_gpio_to_index[GPIO_INDEX_SIZE] = {
	128, 128,   0,   1,   2,   3, 128, 128,   4,   5,   6,   7, 128, 128,
	  8,   9, 128, 128, 128, 128, 128, 128,  10,  11, 128, 128,  12,  13,
	128, 128,  14,  15, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128,  16,  17,  18,  19,  20,  21,  22,  23, 128, 128, 128, 128,
	128, 128, 128, 128,  24,  25, 128, 128, 128,  26,  27,  28,  29,  30,
	 31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42, 128, 128,
	128, 128,  43,  44,  45,  46, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
	 47, 128, 128,  48, 128,  49, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128
};

/**
 * Converts index number of g_gpio into the gpio number
 */
const unsigned char g_index_to_gpio[GPIO_NUM_PINS] = {
	  2,   3,   4,   5,   8,   9,  10,  11,  14,  15,  22,  23,  26,  27,
	 30,  31,  44,  45,  46,  47,  48,  49,  50,  51,  60,  61,  65,  66,
	 67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,
	 81,  86,  87,  88,  89, 112, 115, 117
};

/**
 * Converts PWM index to PWM number
 * e.g index 3 becomes 6 for /sys/class/pwm/pwm6
 */
const unsigned char g_pin_safe_pwm[PWM_NUM_SAFE_PINS] = {
	0, 2, 4, 6, 7
};