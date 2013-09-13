/**
 * @file datapoint.h
 * @purpose Declaration of data handling functions; saving, loading, displaying, selecting.
 */

#ifndef _DATAPOINT_H
#define _DATAPOINT_H

#define DATA_BUFSIZ 10 // Size to use for DataPoint buffers (TODO: Optimise)


#include "common.h"

/** Structure to represent a time, value DataPoint **/
typedef struct
{
	/** Time at which data was taken **/
	double time_stamp; 
	/** Value of data **/
	double value;
} DataPoint;

/** Enum of output format types for DataPoints **/
typedef enum
{
	JSON, // JSON data
	TSV // Tab seperated vector
} DataFormat;

/** 
 * Structure to represent a collection of data. 
 * All operations involving this structure are thread safe.
 * NOTE: It is essentially a wrapper around a binary file.
 */
typedef struct
{
	FILE * read_file; // used for reading
	FILE * write_file; // used for writing
	int num_points; // Number of DataPoints in the file
	char * filename; // Name of the file
	pthread_mutex_t mutex; // Mutex around num_points
} DataFile;


extern void Data_Init(DataFile * df);  // One off initialisation of DataFile
extern void Data_Open(DataFile * df, const char * filename); // Open data file
extern void Data_Close(DataFile * df);
extern void Data_Save(DataFile * df, DataPoint * buffer, int amount); // Save data to file
extern int Data_Read(DataFile * df, DataPoint * buffer, int index, int amount); // Retrieve data from file
extern void Data_PrintByIndexes(DataFile * df, int start_index, int end_index, DataFormat format);  // Print data buffer
extern void Data_PrintByTimes(DataFile * df, double start_time, double end_time, DataFormat format); // Print data between time values
extern int Data_FindByTime(DataFile * df, double time_stamp, DataPoint * closest); // Find index of DataPoint with the closest timestamp to that given

extern void Data_Handler(DataFile * df, FCGIValue * start, FCGIValue * end, DataFormat format, double current_time); // Helper; given FCGI params print data
extern DataFormat Data_GetFormat(FCGIValue * fmt); // Helper; convert human readable format string to DataFormat

#endif //_DATAPOINT_H
