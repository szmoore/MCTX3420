/**
 * @file interferometer.c
 * @purpose Implementation of interferometer related functions
 */

#include "cv.h"
#include "highgui_c.h"
#include "interferometer.h"
#include <math.h>

/** Buffer for storing image data. Stored as a single intensity value for the laser light **/
static CvMat * g_data = NULL;


/** Camera capture pointer **/
static CvCapture * g_capture = NULL;


struct timeval start;

#define PI 3.141592

// For testing purposes
double test_omega = 0.05;
double test_angle = PI/2;
double test_noise[] = {0,0,0.02};
double test_phase = 0;
double test_intensity = 1.0;


static void Interferometer_TestSinusoid()
{
	if (g_capture == NULL)
	{
		g_capture = cvCreateCameraCapture(0);
	}

	// Get image from camera
	IplImage * img = cvQueryFrame(g_capture);

	// Convert to CvMat
	CvMat stub;
	CvMat * background = cvGetMat(img, &stub, 0, 0);
	// ... Honestly, I have no idea what the "stub" is for

	if (g_data == NULL)
	{
		g_data = cvCreateMat(background->rows, background->cols, CV_32FC3);
	}

	//cvShowImage("background", background);

	for (int x = 0; x < g_data->cols-1; ++x)
	{
		for (int y = 0; y < g_data->rows-1; ++y)
		{
			// Calculate pure sine in test direction
			double r = x*cos(test_angle) + y*sin(test_angle);
			double value = 0.5*test_intensity*(1+sin(test_omega*r + test_phase));

			CvScalar s; 
			s.val[0] = 0; s.val[1] = 0; s.val[2] = value;
			CvScalar b = cvGet2D(background, y, x);


			// Add noise & background image

			// Get the order the right way round
			double t = b.val[0];
			b.val[0] = b.val[2];
			b.val[2] = t;
			for (int i = 0; i < 3; ++i)
			{
					
				s.val[i] += (rand() % 1000) * 1e-3 * test_noise[i];
				s.val[i] += b.val[i] / 255; // Camera image is 0-255
			}

			//printf("set %d,%d\n", x, y);

		

			cvSet2D(g_data,y, x, s);
		}
	}	


}


/**
 * Get an image from the Interferometer
 */
static void Interferometer_GetImage()
{


	Interferometer_TestSinusoid();
	//TODO: Implement camera 

}

/**
 * Initialise the Interferometer
 */
void Interferometer_Init()
{
	
	// Make an initial reading (will allocate memory the first time only).
	Interferometer_Read(1); 
}

/**
 * Cleanup Interferometer stuff
 */
void Interferometer_Cleanup()
{
	if (g_data != NULL)
		cvReleaseMat(&g_data);

	if (g_capture != NULL)
		cvReleaseCapture(&g_capture);

}

/**
 * Read the interferometer; gets the latest image, processes it, spits out a single number
 * @param samples - Number of columns to scan (increasing will slow down performance!)
 * @returns Value proportional to the change in interferometer path length since the last call to this function
 */
double Interferometer_Read(int samples)
{




	// Get the latest image
	Interferometer_GetImage();
	// Frequency of the sinusoid
	static double omega = 0;
	// Stores locations of nodes
	static int nodes[MAXNODES];
	// Current phase
	static double phase = 0;

	// Phase
	double cur_phase = 0;

	int xstep = g_data->cols / (samples+1);

	// Used for testing to see where the nodes are identified
	// (Can't modify g_data)
	static CvMat * test_overlay = NULL;
	if (test_overlay == NULL)
	{
		// Creates a memory leak; don't do this in the final version!
		test_overlay = cvCreateMat(g_data->rows, g_data->cols, CV_32FC3);
	}
	cvZero(test_overlay);
	//cvCopy(g_data, test_overlay, NULL);

	// For each column to sample
	for (int x = xstep; x < g_data->cols; x += xstep)
	{
		
		double avg = 0.5; //TODO: Calculate from image
		double threshold_dif = 0; //TODO: Pick this value

		int num_nodes = 0;
		
		// Find nodes
		for (int y = 1; y < g_data->rows-2 && num_nodes < MAXNODES; ++y)
		{
			if (num_nodes == 0 || abs(nodes[num_nodes-1] - y) > 1)
			{
				// A "node" is defined where the ajacent points are on opposite sides of the avg
				double ldif = INTENSITY(cvGet2D(g_data, y-1,x)) - avg;
				double rdif = INTENSITY(cvGet2D(g_data, y+1,x)) - avg;

				// If that is the case, the product of the differences will be negative
				if (ldif * rdif < -threshold_dif)
				{

					nodes[num_nodes++] = y;

					// Put a white line on the overlay to indicate the node was found
					for (int xx = 0; xx < g_data->cols; ++xx)
					{
						CvScalar s; // = cvGet2D(g_data, y, xx);
						s.val[0] = 1; s.val[1] = 1; s.val[2] = 1;
						cvSet2D(test_overlay, y, xx, s);
					}
				}
			}
		}

		// Insufficient nodes found to continue
		if (num_nodes < 2)
		{
			--samples;
			continue;
		}

		// Estimate angular frequency from two nodes TODO: Average between nodes
		double slice_omega = (PI*(num_nodes-1)) / (nodes[num_nodes-1] - nodes[0]);
		//printf("SLICE: %f vs %f\n", slice_omega, test_omega);

		double slice_phase = 0;
		for (int i = 0; i < num_nodes; ++i)
		{
			double this_phase = ((double)(i)*PI - slice_omega * nodes[i]);
			//printf("Node %d gives phase %f\n", i, this_phase);
			slice_phase += this_phase;
		}
		slice_phase /= num_nodes;
		cur_phase += slice_phase;
	}

	// Average over samples
	if (samples == 0)
		return 0;

	cur_phase /= samples;



	// Get phase change since last call, save current phase
	double result = (cur_phase - phase);

	// HACK
	if (abs(result) > 0.5*PI)
	{
		if (result > 0)
			result -= PI;
		else
			result += PI;
	}	
	phase = cur_phase;

	// Display the image with lines to indicate results of data processing
	//cvShowImage("nodes", test_overlay);
	//cvWaitKey(1);
	cvAdd(test_overlay, g_data, test_overlay, NULL);
	cvShowImage("overlay", test_overlay);
	cvWaitKey(1);
	return result;

}

/**
 * For testing purposes
 */
int main(int argc, char ** argv)
{
	//cvNamedWindow( "display", CV_WINDOW_AUTOSIZE );// Create a window for display.
	gettimeofday(&start, NULL);

	//Interferometer_Read(1);
	//exit(EXIT_SUCCESS);

	Interferometer_Init();

	double sum = 0;
	double last_phase = test_phase;

	struct timeval now;
	double time = 0;

	while (time < 20)
	{
		gettimeofday(&now, NULL);
		time = TIMEVAL_DIFF(now, start);
		
		test_phase = 0.5*PI*sin(time);

		
		double delta = Interferometer_Read(1);
		sum += delta;
	
		printf("%f\t%f\t%f\t%f\t%f\n", time, test_phase - last_phase, test_phase, delta, sum);	

		last_phase = test_phase;
		//break;
	}
	
}


