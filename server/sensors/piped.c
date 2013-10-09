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

bool Piped_Read(int id, double * value)
{
	if (g_piped[id].stream == NULL)
		return false;

	static char line[BUFSIZ];
	
	fgets(line, BUFSIZ, g_piped[id].stream);
	int len = strlen(line);
	//Log(LOGDEBUG, "Read %s (%d) chars", line, len);
	while (--len >= 0 && len < BUFSIZ && isspace(line[len]))
	{
		line[len] = '\0';
	}
	char * end = line;
	*value = strtod(line, &end);
	if (*end != '\0')
	{
		Log(LOGERR, "Couldn't interpret %s as double - %s", line, strerror(errno));
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
