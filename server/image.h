/**
 * @file image.h
 * @brief Helper functions for image processing
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include "common.h"

extern void Image_Init();
extern void Image_Handler(FCGIContext * context, char * params); 
extern void Image_Cleanup();

#endif //_IMAGE_H

//EOF
