#ifndef _STRAIN_H
#define _STRAIN_H

#include <stdbool.h>

/**
 * Enum of strain IDs
 */
typedef enum
{
	STRAIN0,
	STRAIN1,	
	STRAIN2,
	STRAIN3
} StrainID;

// Initialise a strain gauge
extern bool Strain_Init(const char * name, int id);
// Read from a strain gauge
extern bool Strain_Read(int id, double * value);

#endif //_STRAIN_H
