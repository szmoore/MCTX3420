/**
 * @file dilatometer.h
 * @brief Declarations for functions to deal with dilatometer
 */

#include "common.h"

//Threshold to determine the edge of the can
#define THRES 230

//Number of samples of the image to take
#define SAMPLES 600

extern void Dilatometer_Init(); // Initialise the dilatometer
extern void Dilatometer_Cleanup(); // Cleanup
extern bool Dilatometer_Read( double * value); // Read the Dilatometer

