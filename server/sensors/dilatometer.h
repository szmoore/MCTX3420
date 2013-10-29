/**
 * @file dilatometer.h
 * @brief Declarations for functions to deal with dilatometer
 */

#include "../common.h"

//Threshold to determine the edge of the can
#define THRES 230

//Number of samples of the image to take
#define SAMPLES 600

extern bool Dilatometer_Init(const char * name, int id); // Initialise the dilatometer
extern bool Dilatometer_Cleanup(int id); // Cleanup
extern bool Dilatometer_Read(int id, double * value); // Read the Dilatometer

