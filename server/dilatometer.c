/**
 * @file dilatometer.c
 * @brief Implementation of dilatometer related functions
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
 * Create a test image
 */
void Dilatometer_TestImage()
{
	
	CvMat *g_dataRGB;
	g_dataRGB = cvCreateMat(480, 640, CV_8UC3); //32

	//Make a rectangle from col=300 to 500, row=150 to 350
	/*for (int x = 100; x < 500; ++x)
	{
		CvScalar s; 
		s.val[0] = 150; s.val[1] = 233; s.val[2] = 244;
 		cvSet2D(g_dataRGB,150,x,s);
		cvSet2D(g_dataRGB,350,x,s);
	}*/
	for (int y = 0; y < 480; ++y)
	{
		CvScalar s; 
		s.val[0] = 200; s.val[1] = 233; s.val[2] = 244;
 		cvSet2D(g_dataRGB,y,100,s);
		cvSet2D(g_dataRGB,y,500,s);
	}
	if (g_data == NULL)
	{
		g_data = cvCreateMat(g_dataRGB->rows,g_dataRGB->cols,CV_8UC1); //IPL_DEPTH_8U?
		cvCvtColor(g_dataRGB,g_data,CV_RGB2GRAY);
	}
}	

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
	//Test image
	Dilatometer_TestImage();
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
		//Log(LOGNOTE, "Number of samples is greater than the dilatometer image height, sampling every row instead.\n");
		samples = height;
	}

	// Stores the width of the can at different sample locations. Not necessary unless we want to store this information
	//double widths[samples];
	// The average width of the can
	double average_width;
	int sample_height;
	printf("here2; %d\n", width);
	for (int i=0; i<samples; i++)
	{
		// Contains the locations of the 2 edges
		double edges[2] = {0.0,0.0};
		printf("edges init %f; %f\n", edges[0], edges[1]);
		int pos = 0;	// Position in the edges array (start at left edge)
		int num = 0;	// Keep track of the number of columns above threshold

		// Determine the position in the rows to find the edges. 
		sample_height = ceil(height * (i + 1) / samples) -1;
		printf("sample height is %d\n", sample_height);

		//CvScalar test = cvGet2D(g_data, 150,300);
		//printf("test is %f,%f,%f,%f\n", test.val[0], test.val[1], test.val[2], test.val[3]);


		for ( int col = 0; col < width; col++)
		{
			//if ( CV_MAT_ELEM( *g_data, double, col, sample_height) > THRES)
			
			//printf("val is %f\n", cvGet2D(g_data, col, sample_height));
			//printf("position is col: %d, row: %d\n",col, sample_height);
			CvScalar value = cvGet2D(g_data, sample_height, col);
			//printf("value is %f\n", value.val[0]);
			if( value.val[0]> THRES)
			{
				edges[pos] += (double) col;
				num++;
				printf("here; %f\n", edges[pos]);
			}
			// If num > 0 and we're not above threshold, we have left the threshold of the edge
			else if( num > 0)
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
		printf("the average width is %f\n", average_width);
		return average_width;
}

/**
 * For testing purposes
 */
int main(int argc, char ** argv)
{
	//cvNamedWindow( "display", CV_WINDOW_AUTOSIZE );// Create a window for display.
	//gettimeofday(&start, NULL);
	
	Dilatometer_Init();

	cvNamedWindow( "display", CV_WINDOW_AUTOSIZE);
	cvShowImage("display", g_data);
	cvWaitKey(0); 	
	Dilatometer_Read(5);
}

