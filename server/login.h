/**
 * @file login.h
 * @brief Declarations of Login related functions
 */

#ifndef _LOGIN_H
#define _LOGIN_H

#include "common.h"

extern void Login_Handler(FCGIContext * context, char * params); // Handle a login request
extern void Logout_Handler(FCGIContext * context, char * params); // Handle a logout request

#endif //_LOGIN_H

//EOF
