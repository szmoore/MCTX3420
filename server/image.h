/**
 * @file image.h
 * @brief Helper functions for image processing
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include "common.h"
#include "cv.h"

extern void Image_Init();
extern void Image_Handler(FCGIContext * context, char * params); 
extern void Image_Cleanup();
extern bool Camera_GetImage(int num, int width, int height,  CvMat * image);

#endif //_IMAGE_H

//EOF
