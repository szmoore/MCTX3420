/**
 * @file microscope.c
 * @purpose Implementation of microscope related functions
 */

#include "cv.h"
#include "highgui_c.h"
#include "microscope.h"
#include <math.h>

// test positions
static double test_left, test_right;

// Canny Edge algorithm variables
int lowThreshold = 30;
int ratio = 3;
int kernel_size = 3;

/** Buffer for storing image data. Stored as a  **/
static CvMat * g_srcRGB  = NULL; // Source Image
static CvMat * g_srcGray = NULL; // Gray scale of source image
static CvMat * g_edges 	 = NULL; // Detected Edges
static CvMat * g_data    = NULL; // Image to mask edges onto


/** Camera capture pointer **/
static CvCapture * g_capture = NULL;

/**
 * Create a test image using left as left edge and right as right edge positions
 */
void Dilatometer_TestImage()
{
	
	g_srcRGB = cvCreateMat(480, 640, CV_8UC3);

	for( int x = 0; x < 640; ++x)
	{
		for (int y = 0; y < 480; ++y)
		{
			CvScalar s; 
			for( int i = 0; i < 3; ++i)
			{
				s.val[i]  =  210 + (rand() % 1000) * 1e-0 - (rand() % 1000) * 1e-0;
				// Produce an exponential decay around left edge
				if( x < test_left)
					s.val[i] *= exp( (x - test_left) / 25);
				else if( x < 320)
					s.val[i] *= exp( (test_left - x) / 25); 
				// Produce an exponential decay around right edge
				else if( x < test_right)
					s.val[i] *= exp( (x - test_right) / 25); 
				else
 					s.val[i] *= exp( (test_right - x) / 25); 				
			}	
			cvSet2D(g_srcRGB,y,x,s);
		//	if( s.val[0] > 200)
		//		printf("row: %d, col: %d, %f\n", y, x, s.val[0]); 
		}
		
	}
	if (g_data == NULL)
	{
		g_data = cvCreateMat(g_srcRGB->rows,g_srcRGB->cols,CV_8UC1); //IPL_DEPTH_8U?
	}
	cvCvtColor(g_srcRGB,g_data,CV_RGB2GRAY);
}	

/**
 * Initialise the dilatometer
 */
void Microscope_Init()
{
	
	// Make an initial reading (will allocate memory the first time only).
	double val;
	Microscope_Read(&val, 1); 
}

/**
 * Cleanup Interferometer stuff
 */
void Microscope_Cleanup()
{
	if (g_data != NULL)
		cvReleaseMat(&g_data);

	if (g_capture != NULL)
		cvReleaseCapture(&g_capture);

}

/**
 * Get an image from the Dilatometer
 */
static void Microscope_GetImage()
{	
	//Need to implement camera
}

void CannyThreshold()
{
	
	if (g_data == NULL)
	{
		g_data = cvCreateMat(g_srcGray->rows,g_srcGray->cols,CV_8UC1);
	}

	if ( g_edges == NULL)
	{
		g_edges = cvCreateMat(g_srcGray->rows,g_srcGray->cols,CV_8UC1);
	}
 	
	//g_data = 0;
	cvShowImage("display", g_srcGray);
	cvWaitKey(0); 	
	// Reduce noise with a kernel 3x3. Input the grayscale source image, output to edges. (0's mean it's determined from kernel sizes)
	cvSmooth( g_srcGray, g_edges, CV_GAUSSIAN, 9, 9 ,0 ,0 );
	
	cvShowImage("display", g_edges);
	cvWaitKey(0); 	
	
	// Find the edges in the image
	lowThreshold = 35;
	cvCanny( g_edges, g_edges, lowThreshold, lowThreshold*ratio, kernel_size );

	cvShowImage("display", g_edges);
	cvWaitKey(0); 	
	
	// Mask the edges over G_data
	//.copyTo( g_data, g_edges);
}

// Test algorithm
static void Microscope_GetImageTest( )
{	
	//Generates Test image
	//Dilatometer_TestImage();
	
	//Load Test image
	g_srcGray = cvLoadImageM ("testimage.jpg",CV_LOAD_IMAGE_GRAYSCALE );
	CannyThreshold();
}


 /**
 * Read the microscope image. The value changed will correspond to the new location of the edge.
 * @param val - Will store the read value if successful
 * @param samples - Number of rows to scan (increasing will slow down performance!)
 * @returns true on successful read
 */
bool Microscope_Read( double * value, int samples)
{
	bool result = false; 
	double average = 0;
	// Get the image from the camera
	Microscope_GetImageTest();
	
	int width = g_edges->cols;
	int height = g_edges->rows;
	
	// If the number of samples is greater than the image height, sample every row
	if( samples > height)
	{
		samples = height;
	}
	
	int sample_height;
	int num_edges = 0;	// Number of edges. if each sample location has an edge, then num_edges = samples

	for (int i=0; i<samples; i++)
	{
		// Determine the position in the rows to find the edges. 
		// This will give you a number of evenly spaced samples
		sample_height = ceil(height * (i + 1) / samples) -1;
		
		// Need to go through each pixel of a row and find all the locations of a line. If there is more than one pixel, average it. note this only works if the canny edge algorithm returns lines about the actual line (no outliers).
		
		int edge_location=0;
		int num=0;
		for ( int col = 0; col < width; col++)
		{
			// Count the number of points
			// Get the threshold of the pixel at the current location
			CvScalar value = cvGet2D(g_edges, sample_height, col);
			//printf("row: %d, col: %d, value: %f\n",sample_height, col, value.val[0]);
			if( value.val[0]> THRES)
			{
				edge_location += col;
				num++;
			}
		}
		if( num > 0)
		{
			average += ( edge_location / num );
			num_edges++;
			printf("average %f\n", average/num_edges);
		}
	}
	if (num_edges > 0)
		average /= num_edges;
	
	if( average > 0)
	{	
		result = true; //Successfully found an edge
		*value = average;
	}
	return result;
}

// Overlays a line over the given edge position
void Draw_Edge(double edge)
{
	CvScalar value;
	value.val[0]=244;
	for( int i = 0; i < g_srcGray->rows; i++)
	{
		cvSet2D(g_edges,i,edge,value);
	}
	cvShowImage("display", g_edges);
	cvWaitKey(0); 	
}

/**
 * For testing purposes
 */
int main(int argc, char ** argv)
{
	//cvNamedWindow( "display", CV_WINDOW_AUTOSIZE );// Create a window for display.
	//gettimeofday(&start, NULL);
	test_left = 100;
	test_right = 500;
	Microscope_Init();
	
	cvNamedWindow( "display", CV_WINDOW_AUTOSIZE);
//	cvShowImage("display", g_data);
//	cvWaitKey(0); 	
	double width;
	
	double edge;
	Microscope_Read(&edge,15);
	//For testing purposes, overlay the given average line over the image
	Draw_Edge(edge);

}

