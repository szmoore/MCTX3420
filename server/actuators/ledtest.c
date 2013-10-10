#include "ledtest.h"

bool Ledtest_Set(int id, double value)
{
	
	FILE *led_handle = NULL;        //code reference: http://learnbuildshare.wordpress.com/2013/05/19/beaglebone-black-controlling-user-leds
	const char *led_format = "/sys/class/leds/beaglebone:green:usr%d/brightness";
	char buf[50];
	bool turn_on = value;

	for (int i = 0; i < 4; i++) 
	{
		snprintf(buf, 50, led_format, i);
		if ((led_handle = fopen(buf, "w")) != NULL)
		{
			if (turn_on)
				fwrite("1", sizeof(char), 1, led_handle);
			else
				fwrite("0", sizeof(char), 1, led_handle);
			fclose(led_handle);
		}
		else
		{
			Log(LOGDEBUG, "LED fopen failed: %s", strerror(errno)); 
			return false;
		}
	}
	return true;
}
