/**
 * @file control.h
 * @brief Header file for control functions
 */
#ifndef _CONTROL_H
#define _CONTROL_H

/**ID codes for all the actuators**/
typedef enum Actuators {ACT_NONE = -1, ACT_PREG = 0, ACT_SOLENOID1} Actuators;
extern void Control_Handler(FCGIContext *context, char *params);

#endif
