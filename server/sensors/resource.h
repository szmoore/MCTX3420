/**
 * @file resource.h
 * @brief Testing sensors; gets rescource usage
 */

#include <stdbool.h>

/**
 * Enum of sensor ids
 */
typedef enum
{
	RESOURCE_CPU_USER,
	RESOURCE_CPU_SYS
} ResourceID;

extern bool Resource_Read(int id, double * value);
