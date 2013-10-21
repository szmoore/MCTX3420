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

// Canny Edge algorithm variables
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
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
	cvCanny( g_edges, g_edges, lowThreshold, lowThreshold*ratio, kernel_size );

	cvShowImage("display", g_edges);
	cvWaitKey(0); 	
	
	// Mask the edges over G_data
	//.copyTo( g_data, g_edges);
}

// Test algorithm
static void Dilatometer_GetImageTest( )
{	
	//Generates Test image
	//Dilatometer_TestImage();
	
	//Load Test image
	g_srcGray = cvLoadImageM ("testimage.jpg",CV_LOAD_IMAGE_GRAYSCALE );
	CannyThreshold();
}


/**
 * Read the dilatometer; gets the latest image, processes it, THEN DOES WHAT
 * @param samples - Number of rows to scan (increasing will slow down performance!)
 * @returns the average width of the can
 */
double Dilatometer_Read(int samples)
{ 	
	//Get the latest image
	//Dilatometer_GetImage();

	Dilatometer_GetImageTest();
	
	int width = g_srcGray->cols;
	int height = g_srcGray->rows;
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
	for (int i=0; i<samples; i++)
	{
		// Contains the locations of the 2 edges
		double edges[2] = {0.0,0.0};
		int pos = 0;	// Position in the edges array (start at left edge)
		int num = 0;	// Keep track of the number of columns above threshold

		// Determine the position in the rows to find the edges. 
		sample_height = ceil(height * (i + 1) / samples) -1;
		//printf("sample height is %d\n", sample_height);

		//CvScalar test = cvGet2D(g_srcGray, 150,300);
		//printf("test is %f,%f,%f,%f\n", test.val[0], test.val[1], test.val[2], test.val[3]);


		for ( int col = 0; col < width; col++)
		{
			CvScalar value = cvGet2D(g_srcGray, sample_height, col);
			if( value.val[0]> THRES)
			{
				edges[pos] += (double) col;
				num++;
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
	average_width /= (double) samples;
	return average_width;
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
	Dilatometer_Init();

//	cvNamedWindow( "display", CV_WINDOW_AUTOSIZE);
//	cvShowImage("display", g_data);
//	cvWaitKey(0); 	
	double width;
	/*for( int i = 0; i < 20; ++i)
	{
		test_left  -= i * (rand() % 1000) * 1e-3;
		test_right += i * (rand() % 1000) * 1e-3;

		//Make sure left and right positions are sane
		if( test_left < 0)
			test_left = 0;
		if( test_right > 639)
			test_right = 639;
		if( test_left > test_right)
		{
			int tmp = test_right;
			test_right = test_left;
			test_left = tmp;
		}

		width = Dilatometer_Read(5);
		cvNamedWindow( "display", CV_WINDOW_AUTOSIZE);
		cvShowImage("display", g_srcGray);
		cvWaitKey(0); 
		double expected = test_right - test_left;
		double perc = 100 * (expected - width) / expected;
		printf("%d: Left: %.4f.    Width: %.4f.\n  Right: %.4f. Expected: %.4f. Percentage: %.4f\n", i, test_left, width, test_right, expected, perc);
	}*/
}

