#include "cv.h"
#include "highgui_c.h"
#include "image.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>

CvCapture *capture;
int captureID = -1;

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
	
	CvMat * g_src = NULL;   // Source Image
	CvMat * g_encoded;   	// Encoded Image

	Camera_GetImage( num, width, height ,g_src); 
	g_encoded = cvEncodeImage("test_encode.jpg",g_src,0);

	Log(LOGNOTE, "Sending image!");
	FCGI_PrintRaw("Content-type: image/jpg\r\n");
	FCGI_PrintRaw("Cache-Control: no-cache, no-store, must-revalidate\r\n\r\n");
	//FCGI_PrintRaw("Content-Length: %d", g_encoded->rows*g_encoded->cols);
	FCGI_WriteBinary(g_encoded->data.ptr,1,g_encoded->rows*g_encoded->cols);
	
	cvReleaseMat(&g_encoded);
	cvReleaseMat(&g_src);
}

 bool Camera_GetImage(int num, int width, int height,  CvMat * image)
 {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Need to use a mutex to ensure 2 captures are not open at once
	pthread_mutex_lock(&mutex);
	bool result = false;

	capture = cvCreateCameraCapture(num);

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, width);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, height);

	IplImage * frame = cvQueryFrame(capture);
	if( frame == NULL)
		return result;

	// Convert the IplImage pointer to CvMat
        CvMat stub;
        image = cvGetMat(frame, &stub, 0, 0);
	if( image == NULL)
		return result;

	// Release the capture and IplImage pointers
	cvReleaseImageHeader(&frame);
	cvReleaseCapture(&capture);

	pthread_mutex_unlock(&mutex);	//Close the mutex
	return true;
}

