#include "log.h"
#include "options.h"
#include <unistd.h>

static int last_len = 0;

void log_print(int level, char * funct, char * fmt, ...)
{
	if (level > options.verbosity)
		return;

	

	char severity[BUFSIZ];
	switch (level)
	{
		case LOGERR:
			sprintf(severity, "ERROR");
			break;
		case LOGWARN:
			sprintf(severity, "WARNING");
			break;
		case LOGNOTE:
			sprintf(severity, "NOTICE");
			break;
		case LOGINFO:
			sprintf(severity, "INFO");
			break;
		default:
			sprintf(severity, "DEBUG");
			break;
	}

	if (funct != NULL)
		last_len = fprintf(stderr, "%s [%d] : %s : %s - ", options.program, getpid(), severity, funct);
	else
	{
		for (int i = 0; i < last_len; ++i);
			fprintf(stderr, " ");
	}
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
}

void error(char * funct, char * fmt, ...)
{
	if (funct != NULL)
		last_len = fprintf(stderr, "%s [%d] : Fatal error in %s - ", options.program, getpid(), funct);
	else
	{
		for (int i = 0; i < last_len; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "Fatal - ");
	}
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}


