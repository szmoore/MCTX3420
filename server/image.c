#include "cv.h"
#include "highgui_c.h"
#include "image.h"
#include <string.h>
#include <stdio.h>

void Image_Handler(FCGIContext * context, const char * params)
{
	static CvCapture * capture = NULL;
	if (capture == NULL)
		capture = cvCreateCameraCapture(0);
	
	static int p[] = {CV_IMWRITE_JPEG_QUALITY, 100, 0};

	IplImage * frame = cvQueryFrame(capture);
	assert(frame != NULL);
	CvMat * jpg = cvEncodeImage(".jpg", frame, p);

	// Will this work?
	Log(LOGNOTE, "Sending image!");
	FCGI_PrintRaw("Content-type: image/jpg\r\n\r\n");
	//FCGI_PrintRaw("Content-Length: %d", jpg->rows*jpg->cols);
	FCGI_WriteBinary(jpg->data.ptr,1,jpg->rows*jpg->cols);
	
}
