#ifndef _CONTROL_H
#define _CONTROL_H

typedef enum Actuators {ACT_NONE = -1, ACT_PREG = 0, ACT_SOLENOID1} Actuators;
extern void Control_Handler(FCGIContext *context, char *params);

#endif
