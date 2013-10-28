/**
 * @file pressure.h
 * @purpose Declarations for Pressure Sensor functions
 */

#ifndef _PRESSURE_H

#include "../common.h"

typedef enum
{
	PRES_HIGH0,
	PRES_HIGH1,
	PRES_LOW0
} PressureId;

extern bool Pressure_Init(const char * name, int id);
extern bool Pressure_Cleanup(int id);
extern bool Pressure_Read(int id, double * value);
extern bool Pressure_Sanity(int id, double value);

#endif //_PRESSURE_H


