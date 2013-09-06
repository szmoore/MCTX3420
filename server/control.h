/**
 * @file control.h
 * @brief Header file for control functions
 */
#ifndef _CONTROL_H
#define _CONTROL_H

/** Number of actuators **/
#define NUMACTUATORS 2

/** List of actuator ids (should be of size NUMACTUATORS) **/
typedef enum ActuatorId {
	ACT_PRESSURE,
	ACT_SOLENOID1
} ActuatorId;

/** Human readable names for the actuator ids **/
extern const char * g_actuator_names[NUMACTUATORS];

/** ID codes for all the actuators **/
extern void Control_Handler(FCGIContext *context, char *params);

#endif
