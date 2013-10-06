/**
 * @file dilatometer.c
 * @purpose Implementation of dilatometer related functions
 */

#include "cv.h"
#include "highgui_c.h"
#include "dilatometer.h"
#include <math.h>

/** Buffer for storing image data. Stored as a  **/
static CvMat * g_data = NULL;


/** Camera capture pointer **/
static CvCapture * g_capture = NULL;


/**
 * Initialise the dilatometer
 */
void Dilatometer_Init()
{
	
	// Make an initial reading (will allocate memory the first time only).
	Dilatometer_Read(1); 
}

/**
 * Cleanup Interferometer stuff
 */
void Dilatometer_Cleanup()
{
	if (g_data != NULL)
		cvReleaseMat(&g_data);

	if (g_capture != NULL)
		cvReleaseCapture(&g_capture);

}

/**
 * Get an image from the Dilatometer
 */
static void Dilatometer_GetImage()
{
	//Need to supply test image

	//Need to implement camera
}
/**
 * Read the dilatometer; gets the latest image, processes it, THEN DOES WHAT
 * @param samples - Number of rows to scan (increasing will slow down performance!)
 * @returns the average width of the can
 */
double Dilatometer_Read(int samples)
{
	//Get the latest image
	Dilatometer_GetImage();

	int width = g_data->cols;
	int height = g_data->rows;
	// If the number of samples is greater than the image height, sample every row
	if( samples > height)
	{
		Log(LOGNOTE, "Number of samples is greater than the dilatometer image height, sampling every row instead.\n");
		samples = height;
	}

	// Stores the width of the can at different sample locations. Not necessary unless we want to store this information
	//double widths[samples];
	// The average width of the can
	double average_width;
	int sample_height;
	for (int i=0; i<samples; i++)
	{
		// Contains the locations of the 2 edges
		double edges[2] = {0};
		int pos = 0;	// Position in the edges array (start at left edge)
		int num = 0;	// Keep track of the number of columns above threshold

		// Determine the position in the rows to find the edges. 
		sample_height = ceil(height * (i + 1) / samples) -1;
		
		for ( int col = 0; col < width; col++)
		{
			if ( CV_MAT_ELEM( *g_data, double, col, sample_height) > THRES)
			{
				edges[pos] += col;
				num++;
			}
			// If num > 0 and we're not above threshold, we have left the threshold of the edge
			else if( num > 0);
			{
				// Find the mid point of the edge
				edges[pos] /= num;
				if( edges[1] == 0) 
				{
					pos = 1;	// Move to the right edge
					num = 0;
				}
				else
					break;		// Exit the for loop
			}
		}
		// Determine the width of the can at this row
		//widths[i] = edges[1] - edges[0];
		average_width += (edges[1] - edges[0]);
	}
		average_width /= samples;
		return average_width;
}

