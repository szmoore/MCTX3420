/**
 * @file pin_test.h
 * @purpose Declarations to allow direct control over pins through FastCGI
 */

#ifndef _PIN_MODULE_H
#define _PIN_MODULE_H

#include "common.h"

extern void Pin_Init();
extern void Pin_Close();
extern void Pin_Handler(FCGIContext *context, char * params);

#endif //_PIN_MODULE_H

//EOF
