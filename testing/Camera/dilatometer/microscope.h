/**
 * @file microscope.h
 * @brief Declarations for functions to deal with microscope
 */

#include "common.h"

//Threshold to determine the edge of the can
#define THRES 230

extern void Microscope_Init(); // Initialise the dilatometer
extern void Microscope_Cleanup(); // Cleanup
extern bool Microscope_Read( double * value, int samples); // Read the Microscope

