/**
 * @file sensor.h
 * @brief Declarations for sensor thread related stuff
 */

#ifndef _SENSOR_H
#define _SENSOR_H

#include "data.h"

/** Number of sensors **/
#define NUMSENSORS 4

/** Safety Values for sensors **/
//TODO: Probably better to use an array instead
#define ANALOG_TEST0_SAFETY 1000
#define ANALOG_TEST1_SAFETY 1000
#define DIGITAL_TEST0_SAFETY 1
#define DIGITAL_TEST1_SAFETY 1


typedef enum SensorId 
{
	ANALOG_TEST0,
	ANALOG_TEST1,
	DIGITAL_TEST0,
	DIGITAL_TEST1
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
	/** Indicates whether the Sensor should record data **/
	bool record_data;
	/** Thread the Sensor is running in **/
	pthread_t thread;
	/** Most recently recorded data **/
	DataPoint newest_data;

} Sensor;


extern void Sensor_Init(); // One off initialisation of *all* sensors

extern void Sensor_StartAll(const char * experiment_name); // Start all Sensors recording data
extern void Sensor_StopAll(); // Stop all Sensors recording data
extern void Sensor_Start(Sensor * s, const char * experiment_name); // Start a sensor recording datas
extern void Sensor_Stop(Sensor * s); // Stop a Sensor from recording data


extern void * Sensor_Loop(void * args); // Main loop for a thread that handles a Sensor
extern bool Sensor_Read(Sensor * s, DataPoint * d); // Read a single DataPoint, indicating if it has changed since the last one
extern void Sensor_CheckData(SensorId id, DataPoint * d); // Check a DataPoint
extern Sensor * Sensor_Identify(const char * str); // Identify a Sensor from a string Id

extern void Sensor_Handler(FCGIContext *context, char * params); // Handle a FCGI request for Sensor data



#endif //_SENSOR_H

