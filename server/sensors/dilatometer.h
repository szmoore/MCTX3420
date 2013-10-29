/**
 * @file dilatometer.h
 * @brief Declarations for functions to deal with dilatometer
 */

#include "../common.h"

//Threshold to determine the edge of the can
#define THRES 230

//Number of samples of the image to take
#define SAMPLES 600

// Canny Edge algorithm variables
#define BLUR 5
#define LOWTHRESHOLD 30
#define RATIO 3
#define KERNELSIZE 3

//Scaling factor required to change from pixels to mm
#define SCALE 1 // Note camera has not been calibrated yet so result will be in pixels

extern bool Dilatometer_Init(const char * name, int id); // Initialise the dilatometer
extern bool Dilatometer_Cleanup(int id); // Cleanup
extern bool Dilatometer_Read(int id, double * value); // Read the Dilatometer

