/**
 * @file interferometer.h
 * @purpose Declarations for functions to deal with interferometer
 */

#include "common.h"

//#define INTENSITY(rgb) ((rgb).val[0]*(rgb).val[0] + (rgb).val[1]*(rgb).val[1] + (rgb).val[2]*(rgb).val[2])
#define INTENSITY(rgb) ((rgb).val[2])

#define MAXNODES 100 //TODO: Choose value based on real data

extern void Interferometer_Init(); // Initialise the interferometer
extern void Interferometer_Cleanup(); // Cleanup
extern double Interferometer_Read(); // Read the interferometer



