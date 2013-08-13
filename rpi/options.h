/**
 * @file options.h
 * @purpose Declaration of structure to handle options passed to program
 */

#include <stdint.h>

typedef struct
{
	const char * program; //name of program
	uint8_t verbosity; // verbosity level
	int port; // port to use for webserver
	int bound_sfd; // socket webserver has bound to
	int sfd; // socket connected to client
	
} Options;



extern Options options;
