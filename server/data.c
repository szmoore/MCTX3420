/**
 * @file data.c
 * @purpose Implementation of data handling functions; saving, loading, displaying, selecting.
 */

#include "data.h"
#include <assert.h> //TODO: Remove asserts

/**
 * One off initialisation of DataFile
 * @param df - The DataFile
 */
void Data_Init(DataFile * df)
{
	// Everything is NULL
	df->filename = NULL;
	df->read_file = NULL;
	df->write_file = NULL;
}

/**
 * Initialise a DataFile from a filename; opens read/write FILE*
 * @param df - DataFile to initialise
 * @param filename - Name of file; overwritten if it exists
 */
void Data_Open(DataFile * df, const char * filename)
{
	assert(filename != NULL);
	assert(df != NULL);

	// Set the filename
 	df->filename = filename;

	// Set number of DataPoints
 	df->num_points = 0; 

	// Set write FILE*
	df->write_file = fopen(filename, "w");
	if (df->write_file == NULL)
	{
		Fatal("Error opening DataFile %s - %s", filename, strerror(errno));
	}
	
	// Set read FILE*
	df->read_file = fopen(filename, "r");
	if (df->read_file == NULL)
	{
		Fatal("Error opening DataFile %s - %s", filename, strerror(errno));
	}


}

/**
 * Close a DataFile
 * @param df - The DataFile to close
 */
void Data_Close(DataFile * df)
{
	assert(df != NULL);

	//TODO: Write data to TSV?

	// Clear the FILE*s
	df->read_file = NULL;
	df->write_file = NULL;

	// Clear the filename
	df->filename = NULL;
}

/**
 * Save DataPoints to a DataFile
 * @param df - The DataFile to save to
 * @param buffer - Array of DataPoint(s) to save
 * @param amount - Number of DataPoints in the buffer
 */
void Data_Save(DataFile * df, DataPoint * buffer, int amount)
{
	assert(df != NULL);
	assert(buffer != NULL);
	assert(amount >= 0);

	// Go to the end of the file
	if (fseek(df->write_file, 0, SEEK_END) < 0)
	{
		Fatal("Error seeking to end of DataFile %s - %s", df->filename, strerror(errno));
	}

	// Attempt to write the DataPoints
	int amount_written = fwrite(buffer, sizeof(DataPoint), amount, df->write_file);
	
	// Check if the correct number of points were written
	if (amount_written != amount)
	{
		Fatal("Wrote %d points instead of %d to DataFile %s - %s", amount_written, amount, df->filename, strerror(errno));
	}

	// Update number of DataPoints
	pthread_mutex_lock(&(df->mutex));
		df->num_points += amount_written;
	pthread_mutex_unlock(&(df->mutex));
	
}

/**
 * Read DataPoints from a DataFile
 * @param df - The DataFile to read from
 * @param buffer - Array to fill with DataPoints
 * @param index - Index to start reading at (inclusive)
 * @param amount - Maximum number of DataPoints to read
 * @returns - Actual number of points read (If smaller than amount, the end of the file was reached)
 */
int Data_Read(DataFile * df, DataPoint * buffer, int index, int amount)
{
	assert(df != NULL);
	assert(buffer != NULL);
	assert(index >= 0);
	assert(amount > 0);
	
	// If we would read past the end of the file, reduce the amount	of points to read
	pthread_mutex_lock(&(df->mutex));
		if (index + amount >= df->num_points)
		{
			Log(LOGDEBUG, "Requested %d points but will only read %d to get to EOF", amount, df->num_points - index);
			amount = df->num_points - index;
		}
	pthread_mutex_unlock(&(df->mutex));

	// Go to position in file
	if (fseek(df->read_file, index*sizeof(DataPoint), SEEK_SET))
	{
		Fatal("Error seeking to position %d in DataFile %s - %s", index, df->filename, strerror(errno));
	}

	// Attempt to read the DataPoints
	int amount_read = fread(buffer, sizeof(DataPoint), amount, df->read_file);

	// Check if correct number of points were read
	if (amount_read != amount)
	{
		Fatal("Read %d points instead of %d from DataFile %s - %s", amount_read, amount, df->filename, strerror(errno));
	}

	return amount;
}

/**
 * Print data points between two indexes using a given format
 * @param df - DataFile to print
 * @param start_index - Index to start at (inclusive)
 * @param end_index - Index to end at (inclusive)
 * @param format - The format to use
 */
void Data_PrintByIndexes(DataFile * df, int start_index, int end_index, DataFormat format)
{
	assert(df != NULL);
	assert(start_index >= 0);
	assert(end_index <= df->num_points-1);

	const char * fmt_string; // Format for each data point
	char seperator; // Character used to seperate successive data points
	
	// Determine what format string and seperator character to use
	switch (format)
	{
		case JSON:
			fmt_string = "[%f,%f]";
			seperator = ',';
			// For JSON we need an opening bracket
			FCGI_PrintRaw("["); 
			break;
		case TSV:
			fmt_string = "%f\t%f";
			seperator = '\n';
			break;
	}

	DataPoint buffer[DATA_BUFSIZ]; // Buffer

	int index = start_index;

	// Repeat until all DataPoints are printed
	while (index <= end_index)
	{
		// Fill the buffer from the DataFile
		int amount_read = Data_Read(df, buffer, index, DATA_BUFSIZ);

		// Print all points in the buffer
		for (int i = 0; i < amount_read; ++i)
		{
			// Print individual DataPoint
			FCGI_PrintRaw(fmt_string, buffer[i].time_stamp, buffer[i].value);
			
			// Last seperator is not required
			if (index+1 < end_index)
				FCGI_PrintRaw("%c", seperator);

			// Advance the position in the DataFile
			++index;
		}
	}
	
	switch (format)
	{
		case JSON:
			// For JSON we need a closing bracket
			FCGI_PrintRaw("]"); 
			break;
		default:
			break;
	}
}

/**
 * Print data points between two time stamps using a given format.
 * Prints nothing if the time stamp
 * @param df - DataFile to print
 * @param start_time - Time to start from (inclusive)
 * @param end_time - Time to end at (inclusive)
 * @param format - The format to use
 */
void Data_PrintByTimes(DataFile * df, double start_time, double end_time, DataFormat format)
{
	assert(df != NULL);
	assert(start_time > 0);
	assert(end_time > 0);
	assert(end_time > start_time);
	
	DataPoint closest;

	// Get starting index
	int start_index = Data_FindByTime(df, start_time, &closest);

	// Start time is greater than most recent time stamp
	if (start_index >= df->num_points-1 && closest.time_stamp < start_time)
	{
		Data_PrintByIndexes(df, 0, 0, format); // Will print "empty" dataset
		return;
	}

	// Get finishing index
	int end_index = Data_FindByTime(df, end_time, &closest);

	// Print data between the indexes
	Data_PrintByIndexes(df, start_index, end_index, format);
}

/**
 * Get the index of the DataPoint closest to a given time stamp
 * @param df - DataFile to search
 * @param time_stamp - The time stamp to search for
 * @param closest - If not NULL, will be filled with the DataPoint chosen
 * @returns index of DataPoint with the *closest* time stamp to that given
 */
int Data_FindByTime(DataFile * df, double time_stamp, DataPoint * closest)
{
	assert(df != NULL);
	assert(time_stamp >= 0);
	assert(closest != NULL);

	DataPoint tmp; // Current DataPoint in binary search

	int lower = 0; // lower index in binary search
	pthread_mutex_lock(&(df->mutex));
		int upper = df->num_points - 1; // upper index in binary search
	pthread_mutex_unlock(&(df->mutex));
	int index = 0; // current index in binary search

	// Commence binary search:
	while (upper - lower > 1)
	{
		// Pick midpoint
		index = lower + ((upper - lower)/2);

		// Look at DataPoint
		if (Data_Read(df, &tmp, index, 1) != 1)
		{
			Fatal("Couldn't read DataFile %s at index %d", df->filename, index);
		}

		// Change search interval to either half appropriately
		if (tmp.time_stamp > time_stamp)
		{
			upper = index;
		}
		else if (tmp.time_stamp < time_stamp)
		{
			lower = index;
		}
	}

	// Store closest DataPoint
	if (closest != NULL)
		*closest = tmp;
	
	return index;
	
}
