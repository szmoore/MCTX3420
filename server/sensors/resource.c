#include "resource.h"

#include "../log.h"
#include "../common.h"

#include <sys/time.h>
#include <sys/resource.h>

bool Resource_Read(int id, double * value)
{
	struct rusage usage;
	int err = getrusage(RUSAGE_SELF, &usage);
	if (err != 0)
	{
		Log(LOGWARN, "Couldn't get resource information - %s", strerror(errno));
	}
	switch (id)
	{
		case RESOURCE_CPU_USER:
			*value = usage.ru_utime.tv_sec + 1e-6*usage.ru_utime.tv_usec;
			break;
		case RESOURCE_CPU_SYS:
			*value = usage.ru_stime.tv_sec + 1e-6*usage.ru_stime.tv_usec;
			break;
		default:
			Log(LOGWARN, "Unknown id %d", id);
			return false;
	}
	return true;
}
