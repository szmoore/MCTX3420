/**
 * @file dilatometer.h
 * @purpose Declarations for functions to deal with dilatometer
 */

#include "common.h"

//Threshold to determine the edge of the can
#define THRES 200

extern void Dilatometer_Init(); // Initialise the dilatometer
extern void Dilatometer_Cleanup(); // Cleanup
extern double Dilatometer_Read( int samples); // Read the dilatometer

