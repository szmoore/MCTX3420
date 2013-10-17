#include "cv.h"
#include "highgui_c.h"
#include "image.h"
#include <string.h>
#include <stdio.h>

CvCapture *captures[2] = {0};

void Image_Handler(FCGIContext * context, char * params)
{
	int num = 0, width = 800, height = 600;
	FCGIValue val[] = {
		{"num", &num, FCGI_INT_T},
		{"width", &width, FCGI_INT_T},
		{"height", &height, FCGI_INT_T}
	};
	if (!FCGI_ParseRequest(context, params, val, 3))
		return;
	else if (num < 0 || num > 1) {
		FCGI_RejectJSON(context, "Invalid capture number");
		return;
	} else if (width <= 0 || height <= 0) {
		FCGI_RejectJSON(context, "Invalid width/height");
		return;
	}

	CvCapture *capture = captures[num];
	if (capture == NULL) {
		capture = cvCreateCameraCapture(num);
		captures[num] = capture;
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, width);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, height);

	static int p[] = {CV_IMWRITE_JPEG_QUALITY, 100, 0};

	IplImage * frame = cvQueryFrame(capture);
	assert(frame != NULL);

//        CvMat stub;
 //       CvMat * background = cvGetMat(frame, &stub, 0, 0);

//	CvMat *cv8u = cvCreateMat(frame->width, frame->height, CV_8U);
//	double min, max;
//	CvPoint a,b;	
//	cvMinMaxLoc(background, &min, &max, &a, &b, 0);
	
//	double ccscale = 255.0/(max-min);
//	double ccshift = -min;
	//cvCvtScale(frame, cv8u, ccscale, ccshift);
	CvMat * jpg = cvEncodeImage(".jpg", frame, p);

	// Will this work?
	Log(LOGNOTE, "Sending image!");
	FCGI_PrintRaw("Content-type: image/jpg\r\n");
	FCGI_PrintRaw("Cache-Control: no-cache, no-store, must-revalidate\r\n\r\n");
	//FCGI_PrintRaw("Content-Length: %d", jpg->rows*jpg->cols);
	FCGI_WriteBinary(jpg->data.ptr,1,jpg->rows*jpg->cols);
	
	cvReleaseMat(&jpg);
	cvReleaseImageHeader(&frame);
}
