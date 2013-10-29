/**
 * @file dilatometer.c
 * @purpose Implementation of dilatometer related functions
 */

#include "cv.h"
#include "highgui_c.h"
#include "dilatometer.h"
#include <math.h>

// test positions
static double test_left, test_right;

// Remembers the last position to measure rate of expansion
static double lastPosition;


/** Buffers for storing image data.  **/
static CvMat * g_srcRGB  = NULL; 	// Source Image
static CvMat * g_srcGray = NULL; 	// Gray scale of source image
static CvMat * g_edges 	 = NULL; 	// Detected Edges

/** Pointers for capturing image **/
//static CvCapture * g_capture = NULL;
//static IplImage * frame  = NULL;	// This is required as you can not use capture with CvMat in C


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
		}
		
	}
	if (g_srcGray == NULL)
	{
		g_srcGray = cvCreateMat(g_srcRGB->rows,g_srcRGB->cols,CV_8UC1);
	}
	cvCvtColor(g_srcRGB,g_srcGray,CV_RGB2GRAY);
}	

/**
 * Cleanup Dilatometer pointers
 */
bool Dilatometer_Cleanup(int id)
{
	//if (g_capture != NULL)
	//	cvReleaseCapture(&g_capture);
	//if (frame != NULL)
	//	cvReleaseImageHeader(&frame);

	//if (g_srcRGB != NULL)
	//	cvReleaseMat(&g_srcRGB);	// Causing run time error in cvReleaseMat
	if (g_srcGray != NULL)
		cvReleaseMat(&g_srcGray);
	if (g_edges != NULL)
		cvReleaseMat(&g_edges);
	return true;
}

/**
 * Get an image from the Dilatometer. Replaced by Camera_GetImage in image.c
 */
/*static bool Dilatometer_GetImage()
{	
	bool result = true;
	// If more than one camera is connected, then input needs to be determined, however the camera ID may change after being unplugged
	if( g_capture == NULL)
	{
		g_capture = cvCreateCameraCapture(0);
		//If cvCreateCameraCapture returns NULL there is an error with the camera
		if( g_capture == NULL)
		{
			result = false;
			return;
		}
	}

	// Get the frame and convert it to CvMat
	frame =  cvQueryFrame(g_capture);
	CvMat stub;
	g_srcRGB = cvGetMat(frame,&stub,0,0);

	if( g_srcRGB == NULL)
		result = false;
	
	// Convert the image to grayscale
	if (g_srcGray == NULL)
	{
		g_srcGray = cvCreateMat(g_srcRGB->rows,g_srcRGB->cols,CV_8UC1);
	}

	cvCvtColor(g_srcRGB,g_srcGray,CV_RGB2GRAY);
	
	return result;
}*/

void CannyThreshold()
{
	// Convert the RGB source file to grayscale
	cvCvtColor(g_srcRGB,g_srcGray,CV_RGB2GRAY);

	if ( g_edges == NULL)
	{
		g_edges = cvCreateMat(g_srcGray->rows,g_srcGray->cols,CV_8UC1);
	}
 	
	// Commented out lines are used during testing to show the image to screen, can also save the test images
	//cvShowImage("display", g_srcGray);
	//cvWaitKey(0); 	
	
	// Reduce noise with a kernel blurxblur. Input the grayscale source image, output to edges. (0's mean it's determined from kernel sizes)
	cvSmooth( g_srcGray, g_edges, CV_GAUSSIAN, BLUR, BLUR ,0 ,0 );
	
	//Save the image
	//cvSaveImage("test_blurred.jpg",g_edges,0);

	//cvShowImage("display", g_edges);
	//cvWaitKey(0); 

	// Find the edges in the image
	cvCanny( g_edges, g_edges, LOWTHRESHOLD, LOWTHRESHOLD*RATIO, KERNELSIZE );
	
	//Save the image
	//cvSaveImage("test_edge.jpg",g_edges,0);

	//cvShowImage("display", g_edges);
	//cvWaitKey(0); 	

}

 /**
 * Read the dilatometer image. The value changed will correspond to the rate of expansion. If no edge is found then 
 * @param val - Will store the read value if successful
 * @param samples - Number of rows to scan (increasing will slow down performance!)
 * @returns true on successful read
 */
bool Dilatometer_GetExpansion( int id, double * value, int samples)
{
	bool result = false; 
	double average = 0;
	// Get the image from the camera
	result = Camera_GetImage( 0, 1600, 1200 ,&g_srcRGB); // Get a 1600x1200 image and place it into src

	// If an error occured when capturing image then return
	if (!result)
		return result;

	// Apply the Canny Edge theorem to the image
	CannyThreshold();

	int width = g_edges->cols;
	int height = g_edges->rows;
	
	// If the number of samples is greater than the image height, sample every row
	if( samples > height)
	{
		samples = height;
	}
	
	int sample_height;
	int num_edges = 0;	// Number of edges found. if each sample location has an edge, then num_edges = samples

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
		}
	}
	if (num_edges > 0)
		average /= num_edges;
	else
		return result; 	// As no edges were found
	
	if( average > 0)
	{	
		result = true; // Successfully found an edge
		// If the experiment has already been initialised
		switch (id)
		{
			case DIL_POS:
				*value = average*SCALE;
				return result;
			case DIL_DIFF:
				if( lastPosition > 0)
				{	
					// Find the rate of expansion and convert to mm. Will give a negative result for compression.
					*value = (average - lastPosition) * SCALE;
					lastPosition = average;	// Current position now becomes the last position
				}
				return result;
			default:
				return false;  		}
	}
	return result;
}

 /**
 * Read the dilatometer image. The value changed will correspond to the new location of the edge.
 * @param val - Will store the read value if successful
 * @returns true on successful read
 */
bool Dilatometer_Read(int id, double * value)
{
	bool result = Dilatometer_GetExpansion(id, value, SAMPLES);
	return result;
}

/**
 * Initialise the dilatometer
 */
bool Dilatometer_Init(const char * name, int id)
{
	// Make an initial reading (will allocate memory the first time only).
	double val;
	lastPosition = 0;  // Reset the last position
	bool result = Dilatometer_GetExpansion(DIL_POS, &val, 1); 
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
	//cvSaveImage("test_edge_avg.jpg",g_edges,0);
}

/* // Test algorithm
static void Dilatometer_GetImageTest( )
{	
	//Generates Test image
	//Dilatometer_TestImage();
	
	//Load Test image
	g_srcGray = cvLoadImageM ("testimage4.jpg",CV_LOAD_IMAGE_GRAYSCALE );
}*/

/**
 * For testing purposes
 */
/*int main(int argc, char ** argv)
{
	//cvNamedWindow( "display", CV_WINDOW_AUTOSIZE );// Create a window for display.
	//gettimeofday(&start, NULL);
	test_left = 100;
	test_right = 500;
	Dilatometer_Init();
	
	cvNamedWindow( "display", CV_WINDOW_AUTOSIZE);
	//double width;
	
	double edge;
	Dilatometer_GetEdge(&edge,20000);
	//For testing purposes, overlay the given average line over the image
	//Draw_Edge(edge);
	
	cvDestroyWindow("display");

	Dilatometer_Cleanup();
}*/

