/**
 * @file sensor.h
 * @brief Declarations for sensor thread related stuff
 */

#ifndef _SENSOR_H
#define _SENSOR_H

/** Number of data points to keep in sensor buffers **/
#define SENSOR_DATABUFSIZ 10
/** Size of the query buffer. @see Sensor_Handler **/
#define SENSOR_QUERYBUFSIZ 10

/** Number of sensors **/
#define NUMSENSORS 4

/** Safety Values for sensors **/
#define ANALOG_TEST0_SAFETY 1000
#define ANALOG_TEST1_SAFETY 1000
#define DIGITAL_TEST0_SAFETY 1
#define DIGITAL_TEST1_SAFETY 1


typedef enum SensorId {
	ANALOG_TEST0,
	ANALOG_TEST1,
	DIGITAL_TEST0,
	DIGITAL_TEST1
} SensorId;

/** Human readable names for the sensors **/
extern const char * g_sensor_names[NUMSENSORS];

/** Structure to represent data recorded by a sensor at an instant in time **/
typedef struct
{
	/** Time at which data was taken **/
	double time_stamp; 
	/** Value of data **/
	double value;
} DataPoint;

/** Structure to represent a sensor **/
typedef struct
{
	/** ID number of the sensor **/
	SensorId id;
	/** Buffer to store data from the sensor **/
	DataPoint buffer[SENSOR_DATABUFSIZ];
	/** Index of last point written in the data buffer **/
	int write_index;
	/** Number of points read **/
	long points_read;
	/** Binary file to write data into when buffer is full **/
	FILE * file;
	/** Thread running the sensor **/
	pthread_t thread;
	/** Mutex to protect access to stuff **/
	pthread_mutex_t mutex;

	
} Sensor;




extern void Sensor_Spawn(); // Initialise sensor
extern void Sensor_Join(); //Join sensor threads
extern void * Sensor_Main(void * args); // main loop for sensor thread; pass a Sensor* cast to void*

extern int Sensor_Query(Sensor * s, DataPoint * buffer, int bufsiz); // fill buffer with sensor data

extern void Sensor_Handler(FCGIContext *context, char * params);

#endif //_SENSOR_H

