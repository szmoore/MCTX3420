/**
 * @file sensor.h
 * @brief Declarations for sensor thread related stuff
 */

#ifndef _SENSOR_H
#define _SENSOR_H

#include "data.h"
#include "device.h"


/** 
 * Maximum number of sensors program can be compiled with
 * (If you get an error "Increase SENSORS_MAX from %d" this is what it refers to)
 */
#define SENSORS_MAX 10
extern int g_num_sensors; // in sensor.c


/** Structure to define the warning and error thresholds of the sensors **/
//TODO: Replace with a call to an appropriate "Sanity" function? (see the actuator code)
typedef struct
{
	/** Maximum safe value **/
	double max_error;
	/** Minimum safe value **/
	double min_error;
	/** Maximum value before a warning is reported **/
	double max_warn;
	/** Minimum value before a warning is reported **/
	double min_warn;
} SensorThreshold;

/** Structure to represent a sensor **/
typedef struct
{
	/** ID number of the sensor **/
	int id;
	/** User defined ID number **/
	int user_id;
	/** DataFile to store sensor values in **/
	DataFile data_file;
	/** Indicates whether the Sensor is active or not **/
	bool activated;
	/** Thread the Sensor is running in **/
	pthread_t thread;
	/** Function to read the sensor **/
	ReadFn read;
	/** Function to initialise the sensor **/
	InitFn init;
	/** Function to cleanup the sensor **/
	CleanFn cleanup;
	/** Function to sanity check the sensor readings **/
	SanityFn sanity;
	/** Human readable name of the sensor **/
	const char * name;
	/** Sampling rate **/
	struct timespec sample_time;
	/** Number of averages per sample **/
	int averages;
	/** Current data **/
	DataPoint current_data;

	/** Summed data **/
	DataPoint averaged_data;
	/** Number of points read so far before applying average **/
	int num_read;


	
} Sensor;



extern void Sensor_Init(); // One off initialisation of *all* sensors
extern void Sensor_Cleanup(); // Cleanup all sensors

extern void Sensor_SetModeAll(ControlModes mode, void * arg);
extern void Sensor_SetMode(Sensor * s, ControlModes mode, void * arg);

extern void * Sensor_Loop(void * args); // Main loop for a thread that handles a Sensor
//extern bool Sensor_Read(Sensor * s, DataPoint * d); // Read a single DataPoint, indicating if it has changed since the last one
extern Sensor * Sensor_Identify(const char * str); // Identify a Sensor from a string

extern void Sensor_Handler(FCGIContext *context, char * params); // Handle a FCGI request for Sensor data

extern DataPoint Sensor_LastData(int id);

extern const char * Sensor_GetName(int id);
extern DataFile * Sensor_GetFile(int id);



#endif //_SENSOR_H


