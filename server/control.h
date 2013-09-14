/**
 * @file control.h
 * @brief Header file for control functions
 */
#ifndef _CONTROL_H
#define _CONTROL_H

/** ID codes for all the actuators **/
extern void Control_Handler(FCGIContext *context, char *params);
extern bool Control_Start(const char *experiment_name);
extern void Control_Pause();
extern bool Control_Stop();
extern bool Control_Lock();
extern void Control_Unlock();

#endif
