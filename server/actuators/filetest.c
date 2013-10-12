#include "filetest.h"

static FILE * f = NULL;
bool Filetest_Init(const char * name, int id)
{
	f = fopen(name, "w");
	setbuf(f, NULL); // Unbuffer
	return (f != NULL);
}

bool Filetest_Set(int id, double value)
{
	Log(LOGDEBUG, "Writing %lf to file", value);
	return (fprintf(f, "%lf\n", value) > 1);
}

bool Filetest_Cleanup(int id)
{
	return (fclose(f) == 0);
}

bool Filetest_Sanity(int id, double value)
{
	return (abs(value) <= 1e4);
}
