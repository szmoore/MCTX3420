
/**
 * @file sensor.h
 * @purpose Declarations for sensor thread related stuff
 */



#ifndef _SENSOR_H
#define _SENSOR_H

#include "common.h"

/** Number of data points to keep in sensor buffers **/
#define SENSOR_DATABUFSIZ 10


/** Number of sensors **/
#define NUMSENSORS 1

/** Structure to represent data recorded by a sensor at an instant in time **/
typedef struct
{
	/** Time at which data was taken **/
	float time;
	/** Value of data **/
	float value;
} DataPoint;

/** Structure to represent a sensor **/
typedef struct
{
	/** ID number of the sensor **/
	enum {SENSOR_TEST0=0, SENSOR_TEST1=1} id;
	/** Buffer to store data from the sensor **/
	DataPoint buffer[SENSOR_DATABUFSIZ];
	/** Index of last point written in the data buffer **/
	int write_index;
	/** Offset position in binary file for query thread to read from**/
	int read_offset;
	/** File to write data into when buffer is full **/
	FILE * file;
	/** Thread running the sensor **/
	pthread_t thread;
	/** Mutex to protect access to stuff **/
	pthread_mutex_t mutex;

	
} Sensor;

/** Array of Sensors **/
extern Sensor g_sensors[];

extern void Sensor_Init(Sensor * s, int id); // Initialise sensor
extern void * Sensor_Main(void * args); // main loop for sensor thread; pass a Sensor* cast to void*


#endif //_SENSOR_H

