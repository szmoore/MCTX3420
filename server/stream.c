#include "cv.h"
#include "highgui_c.h"
#include <string.h>
#include <stdio.h>

/*-------------------------------------------------------------------

compile with:
-I/usr/include/opencv -I/usr/include/opencv2/highgui -L/usr/lib -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_imgproc

--------------------------------------------------------------------*/

int storeFrame( CvCapture* capture)
{
	IplImage *frame;
	
	int p[3];
	p[0] = CV_IMWRITE_JPEG_QUALITY;
  	p[1] = 50;	//quality value. 0-100
  	p[2] = 0;
		
	frame = cvQueryFrame(capture);
	if( frame == NULL)
		return 0;	//error
	cvSaveImage("../web/images/test.JPG",frame,p);
	cvReleaseImageHeader(&frame);
	return 1;
}

int main (int argc, char** argv)
{
	CvCapture* capture;
	
	//Get capture structure for camera, -1 refers to any camera device.
	//If multiple cameras used, need to use specific camera ID
	capture = cvCreateCameraCapture(-1);
	//If cvCreateCameraCapture returns NULL there is an error with the camera
	if( capture == NULL)
		return -1;

	while(1)
	{
		if( !storeFrame( capture))
			return -1;
		//sleep(1);	//for now just to make the camera take 1 shot a second, as that's all my camera seems to be able to take/save (camera or function causing this? is 1 second per frame enough?)
	}
	
	//Need to determine how the function is called in respect to system. just leave it running with a while loop? will something turn it on and off? will the function be called once from elsewhere?

	cvReleaseCapture( &capture);
}

