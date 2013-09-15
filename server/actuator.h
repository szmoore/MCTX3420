/**
 * @file actuator.h
 * @brief Declarations for actuator control
 */

#ifndef _ACTUATOR_H
#define _ACTUATOR_H

#include "common.h"
#include "data.h"

//NOTE: Functionality is very similar to Sensor stuff
//		BUT it's probably very unwise to try and generalise Sensors and Actuators to the same thing (ie: Device)
//		Might be OK in C++ but not easy in C

/** Number of actuators **/
#define NUMACTUATORS 2

/** List of actuator ids (should be of size NUMACTUATORS) **/
typedef enum
{
	ACTUATOR_TEST0,
	ACTUATOR_TEST1
} ActuatorId;

/** Human readable names for the Actuators **/
extern const char * g_actuator_names[NUMACTUATORS];

/** Control structure for Actuator setting **/
typedef struct
{
	//TODO: Add functionality as needed
	/** Simple value for Actuator **/
	double value;
} ActuatorControl;

typedef struct
{
	/** ID number of the actuator **/
	ActuatorId id;
	/** Control parameters for the Actuator **/
	ActuatorControl control;
	/** Flag indicates if ActuatorControl has been changed **/
	bool control_changed;
	/** DataFile to store actuator settings **/
	DataFile data_file;
	/** Thread the Actuator is controlled by **/
	pthread_t thread;
	/** Mutex around ActuatorControl **/
	pthread_mutex_t mutex;
	/** Used to wake up Actuator control thread **/
	pthread_cond_t cond;
	/** Indicates whether the Actuator is running **/
	bool activated;
	/** Indicates whether the Actuator can be actuated or not **/
	bool allow_actuation;

} Actuator;

extern void Actuator_Init(); // One off initialisation of *all* Actuators

extern void Actuator_SetModeAll(ControlModes mode, void *arg);
extern void Actuator_SetMode(Actuator * a, ControlModes mode, void *arg);

extern void * Actuator_Loop(void * args); // Main loop for a thread that handles an Actuator
extern void Actuator_SetValue(Actuator * a, double value); // Set an actuator by value
extern void Actuator_SetControl(Actuator * a, ActuatorControl * c); // Set the control for an Actuator
extern Actuator * Actuator_Identify(const char * str); // Identify a Sensor from a string Id

extern void Actuator_Handler(FCGIContext *context, char * params); // Handle a FCGI request for Actuator control

#endif //_ACTUATOR_H

//EOF
