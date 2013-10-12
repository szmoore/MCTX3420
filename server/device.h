/**
 * @file device.h
 * @brief Declare code/typedefs common to both Sensors and Actuators
 */
#ifndef _DEVICE_H
#define _DEVICE_H

/** Function pointer for sensor reading **/
typedef bool (*ReadFn)(int, double *);
/** Function pointer for actuator setting **/
typedef bool (*SetFn)(int, double);
/** Function pointer for sensor initialisation **/
typedef bool (*InitFn)(const char *, int);
/** Function pointer for sensor cleanup **/
typedef bool (*CleanFn)(int);
/** Function to check the sanity of a value **/
typedef bool (*SanityFn)(int, double);

#endif //_DEVICE_H
