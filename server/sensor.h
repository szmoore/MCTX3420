/**
 * @file sensor.h
 * @brief Declarations for sensor thread related stuff
 */

#ifndef _SENSOR_H
#define _SENSOR_H

#include "data.h"



/** Number of sensors **/
#define NUMSENSORS 10

/** Sensor ids - there should be correspondence with the names in g_sensor_names **/
typedef enum SensorId 
{
	STRAIN0,
	STRAIN1,
	STRAIN2,
	STRAIN3,
	PRESSURE0,
	PRESSURE1,
	PRESSURE_FEEDBACK,
	MICROPHONE,
	ENCLOSURE.
	DILATOMETER
} SensorId;



/** Human readable names for the sensors **/
extern const char * g_sensor_names[NUMSENSORS];

/** Structure to represent a sensor **/
typedef struct
{
	/** ID number of the sensor **/
	SensorId id;
	/** DataFile to store sensor values in **/
	DataFile data_file;
	/** Indicates whether the Sensor is active or not **/
	bool activated;
	/** Thread the Sensor is running in **/
	pthread_t thread;
	/** Most recently recorded data **/
	DataPoint newest_data;
	
	
} Sensor;

// Structure to define the warning and error thresholds of the sensors
typedef struct
{
	double max_error;
	double min_error;
	double max_warn;
	double min_warn;
} SensorThreshold;

extern void Sensor_Init(); // One off initialisation of *all* sensors

extern void Sensor_SetModeAll(ControlModes mode, void * arg);
extern void Sensor_SetMode(Sensor * s, ControlModes mode, void * arg);

extern void * Sensor_Loop(void * args); // Main loop for a thread that handles a Sensor
extern bool Sensor_Read(Sensor * s, DataPoint * d); // Read a single DataPoint, indicating if it has changed since the last one
extern void Sensor_CheckData(SensorId id, double value); // Check a DataPoint
extern Sensor * Sensor_Identify(const char * str); // Identify a Sensor from a string Id

extern void Sensor_Handler(FCGIContext *context, char * params); // Handle a FCGI request for Sensor data

#endif //_SENSOR_H


