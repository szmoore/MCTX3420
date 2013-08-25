#include "cv.h"
#include "highgui_c.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

int storeFrame( CvCapture* capture)
{
	IplImage *frame;
	time_t rawtime;	// time given in seconds since jan 1st 1970
	struct tm *timeInfo;// time structure containing current time info
	int buf = 100;
	char timestamp[buf];
	char filepath[buf];	// filepath to save the image to

	//USING char *filepath creates seg fault. need to define bufsize, how big?

	time(&rawtime);
	timeInfo = localtime(&rawtime);
	snprintf(timestamp,buf-1,"%d.%d.%d_%d.%d.%d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday,timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
	snprintf(filepath,buf-1,"images/image_%s.JPG",timestamp);

	/*int p[3];
	p[0] = CV_IMWRITE_JPEG_QUALITY;
  	p[1] = 10;
  	p[2] = 0;*/
		
	frame = cvQueryFrame(capture);
	if( frame == NULL)
		return 0;	//error
	cvSaveImage(filepath,frame,0);
	
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

	int i;
	for( i=0;i<10;i++)
	{
		if( !storeFrame( capture))
			return -1;
		sleep(1);	//for now just to make the camera take 1 shot a second, as that's all my camera seems to be able to take/save (camera or function causing this? is 1 second per frame enough?)
	}
	
	//Need to determine how the function is called in respect to system. just leave it running with a while loop? will something turn it on and off? will the function be called once from elsewhere?

	cvReleaseCapture( &capture);
}

