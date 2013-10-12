/**
 * @file piped.h
 * @brief Sensor run by an external process and sent to this one through a pipe
 * PURELY INCLUDED FOR TESTING PURPOSES
 *	This will work with any sensor that can unbuffer stdout
 * ... So, although it's not recommended, you could write a sensor purely in something like python
 *	   The FastCGI process will handle all the time stamps and stuff
 */

#include "../log.h"
#include "../common.h"

#include "piped.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <ctype.h>

#include <float.h> // Defines DBL_MAX_10_EXP


typedef struct
{
	int pid;
	int sv[2];
	FILE * stream;
	
} Piped;

static Piped g_piped[PIPED_MAX];
static int g_num_piped = 0;

bool Piped_Init(const char * name, int id)
{
	if (++g_num_piped > PIPED_MAX)
	{
		Fatal("Too many sensors; Increase PIPED_MAX from %d in piped.h and recompile", PIPED_MAX);
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, g_piped[id].sv) != 0)
			Fatal("socketpair failed - %s", strerror(errno));

	g_piped[id].pid = fork();
	if (g_piped[id].pid == 0)
	{
		dup2(g_piped[id].sv[0], fileno(stdin));
		dup2(g_piped[id].sv[0], fileno(stdout));

		if (access(name, X_OK) == 0) //Check we STILL have permissions to start the file
		{
			execl(name, name, (char*)(NULL)); ///Replace process with desired executable
			//execv(executablePath,arguments); ///Replace process with desired executable
		}
		else
		{
			Fatal("Can't execute file %s", name);
		}
		Fatal("execl error - %s", strerror(errno));
	}
	else
	{
		g_piped[id].stream = fdopen(g_piped[id].sv[1], "r");
		setbuf(g_piped[id].stream, NULL);
	}
	return true;
	
}
/**
 * This function looks evil, but I swear it's legit
 * @param id - The Piped process to read from
 * @param value - Stores the value read (if successful)
 * @returns true on success, false on failure
 */
bool Piped_Read(int id, double * value)
{
	if (g_piped[id].stream == NULL)
		return false;

	// So need a buffer size big enough to fit all doubles but not too much bigger
	static char buf[DBL_MAX_10_EXP+1]; 

	// Using BUFSIZ is a bad idea, since we want to read near the end of the file

	// Seek back from the end of the file
	fseek(g_piped[id].stream, -DBL_MAX_10_EXP, SEEK_END);	
	int len = fread(buf, 1, DBL_MAX_10_EXP, g_piped[id].stream); // Fill the buffer, note the length count
	int count = 0;
	int i = len-1;
	for (i = len-1; i >= 0; --i) // Search for the last non-empty line in the buffer
	{
		if (buf[i] == '\n')
		{
			if (count++ > 1)
			{
				++i;
				break;
			}
			else
				buf[i] = '\0';
		}
	}
	// Now sscanf a double from the string
	if (sscanf(buf+i, "%lf", value) != 1)
	{
		Log(LOGDEBUG, "Can't interpret %s as double", buf);
		return false;
	}
	return true;
}

bool Piped_Cleanup(int id)
{
	fclose(g_piped[id].stream);
	if (kill(g_piped[id].pid, 15) != 0)
	{
		Log(LOGWARN, "Couldn't kill piped %d - %s", g_piped[id].pid, strerror(errno));
		return false;
	}
	return true;
}
