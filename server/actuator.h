/**
 * @file actuator.h
 * @brief Declarations for actuator control
 */

#ifndef _ACTUATOR_H
#define _ACTUATOR_H

#include "common.h"
#include "data.h"
#include "device.h"


/** 
 * Maximum number of actuators program can be compiled with
 * (If you get an error "Increase ACTUATORS_MAX from %d" this is what it refers to)
 */
#define ACTUATORS_MAX 5
extern int g_num_actuators; // in actuator.c



/** Control structure for Actuator setting **/
typedef struct
{
	//TODO: Add functionality as needed
	// Currently implements a simple piecewise step increase
	// Would be cool to have a function specified as a string... eg: "1.0 + 0.5*s^2" with "s" the step number, and then give "stepwait" and "steps"
	//	... But that, like so many things, is probably overkill
	/** Current value of Actuator **/
	double start;
	/** Time to maintain Actuator at each value **/
	double stepwait;
	/**	Amount to increase/decrease Actuator on each step **/
	double stepsize;
	/** Number of steps still to perform **/
	int steps; // Note that after it is first set, this will be decremented until it is zero

} ActuatorControl;

typedef struct
{
	/** ID number of the actuator **/
	int id;
	/** User ID number **/
	int user_id;
	/** Name **/
	const char * name;
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
	/** Initialisation function **/
	InitFn init;
	/** Set function **/
	SetFn set;
	/** Sanity check function **/
	SanityFn sanity;
	/** Cleanup function **/
	CleanFn clean;
	
} Actuator;

extern void Actuator_Init(); // One off initialisation of *all* Actuators

extern void Actuator_SetModeAll(ControlModes mode, void *arg);
extern void Actuator_SetMode(Actuator * a, ControlModes mode, void *arg);

extern void * Actuator_Loop(void * args); // Main loop for a thread that handles an Actuator
extern void Actuator_SetValue(Actuator * a, double value); // Set an actuator by value
extern void Actuator_SetControl(Actuator * a, ActuatorControl * c); // Set the control for an Actuator
extern Actuator * Actuator_Identify(const char * str); // Identify a Sensor from a string Id

extern void Actuator_Handler(FCGIContext *context, char * params); // Handle a FCGI request for Actuator control
extern const char * Actuator_GetName(int id);

#endif //_ACTUATOR_H

//EOF
