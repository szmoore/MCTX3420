/**
 * @file control.h
 * @brief Header file for control functions
 */
#ifndef _CONTROL_H
#define _CONTROL_H

typedef enum ControlModes {
	CONTROL_START,
	CONTROL_PAUSE,
	CONTROL_RESUME,
	CONTROL_STOP,
	CONTROL_EMERGENCY
} ControlModes;

/** ID codes for all the actuators **/
extern void Control_Handler(FCGIContext *context, char *params);
extern const char* Control_SetMode(ControlModes desired_mode, void * arg);
extern ControlModes Control_GetMode();
extern const char * Control_GetModeName(ControlModes mode);
//extern bool Control_Lock();
//extern void Control_Unlock();
extern const struct timeval* Control_GetStartTime();

#endif
