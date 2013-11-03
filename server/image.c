#include "cv.h"
#include "highgui_c.h"
#include "image.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>

static CvCapture * g_capture = NULL;
static int g_captureID = -1;

/**
 * Image stream handler. Returns an image to the user.
 * @param context The context to work in
 * @param params User specified parameters
 */
void Image_Handler(FCGIContext * context, char * params)
{

	int num = 0, width = 1600, height = 1200;	// Set Default values
	FCGIValue val[] = {
		{"num", &num, FCGI_INT_T},
		{"width", &width, FCGI_INT_T},
		{"height", &height, FCGI_INT_T}
	};
	if (!FCGI_ParseRequest(context, params, val, 3))	// Populate val
		return;
	// Ensure the camera id is 0 or 1. Even though we plan to only have 1 camera attached at a time, this will allow 2. increase
	else if (num < 0 || num > 1) {				
		FCGI_RejectJSON(context, "Invalid capture number");
		return;
	// Ensure valid widths
	} else if (width <= 0 || height <= 0) {
		FCGI_RejectJSON(context, "Invalid width/height");
		return;
	}
	
	IplImage * src = NULL;   // Source Image
	CvMat * encoded = NULL;   	// Encoded Image

	Camera_GetImage( num, width, height ,&src); 

	Log(LOGDEBUG, "About to encode");
	encoded = cvEncodeImage(".jpg",src,0);
	Log(LOGDEBUG, "Encoded");

	Log(LOGNOTE, "Sending image!");
	FCGI_PrintRaw("Content-type: image/jpg\r\n");
	FCGI_PrintRaw("Cache-Control: no-cache, no-store, must-revalidate\r\n\r\n");
	//FCGI_PrintRaw("Content-Length: %d", g_encoded->rows*g_encoded->cols);
	FCGI_WriteBinary(encoded->data.ptr,1,encoded->rows*encoded->cols);
	
	cvReleaseMat(&encoded);
	cvReleaseImageHeader(&src);
}
	
/**
 * Attempts to get an image from a camera
 * @param num - Camera id
 * @param width - Width to force
 * @param height - Height to force
 * @param frame - Pointer to IplImage* to set with result
 * @returns true on success, false on error 
 */
 bool Camera_GetImage(int num, int width, int height,  IplImage ** frame)
 {
	Log(LOGDEBUG, "Called with arguments num=%d width=%d height=%d frame=%p", num,width,height, frame);
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Need to use a mutex to ensure 2 captures are not open at once
	pthread_mutex_lock(&mutex);
	bool result = false;

	if( g_capture == NULL)
	{
		g_capture = cvCreateCameraCapture(num);
		g_captureID = num;
	}
	else if( num != g_captureID)
	{
		cvReleaseCapture(&g_capture);
		g_capture = cvCreateCameraCapture(num);	
		g_captureID = num;
	}

	if (g_capture != NULL)
	{

		cvSetCaptureProperty(g_capture, CV_CAP_PROP_FRAME_WIDTH, width);
		cvSetCaptureProperty(g_capture, CV_CAP_PROP_FRAME_HEIGHT, height);

		*frame = cvQueryFrame(g_capture);
		result = (*frame != NULL);

	//cvShowImage("display", *image);
	//cvWaitKey(0); 
	//cvSaveImage("test.jpg",*image,0);
		
		Log(LOGDEBUG, "At end of mutex");
	}

	pthread_mutex_unlock(&mutex);	//Close the mutex

	//NOTE: Never have a "return" statement before the mutex is unlocked; it causes deadlocks!
	return result;
}

/**
 * Executed on cleanup. Releases the OpenCV Capture structs.
 */
void Image_Cleanup()
{
	// Release the capture and IplImage pointers
	//cvReleaseImageHeader(&g_frame);
	cvReleaseCapture(&g_capture);
}

