#ifndef _PIPED_H
#define _PIPED_H

/**
 * @file piped.h
 * @brief Sensor run by an external process and sent to this one through a pipe
 *		... This is here as an example so that if people really hate our program they can still use the FastCGI API but make an entirely seperate sensor program
 */

#define PIPED_MAX 1

extern bool Piped_Init(const char * name, int id);
extern bool Piped_Read(int id, double * value);
extern bool Piped_Cleanup(int id);
#endif //_PIPED_H
