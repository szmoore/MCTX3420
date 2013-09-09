DataPoint * GetData(int sensor_id, DataPoint * d)
{	
	//TODO: We should ensure the time is *never* allowed to change on the server if we use gettimeofday
	//		Another way people might think of getting the time is to count CPU cycles with clock()
	//		But this will not work because a) CPU clock speed may change on some devices (RPi?) and b) It counts cycles used by all threads
	gettimeofday(&(d->time_stamp), NULL);
	
	switch (sensor_id)
	{
		//TODO: test buffer size on actual hardware
		//		maybe map the sensor path to an array/structure that can be looked up via sensor_id?
		//		not sure about the best place to do unit conversions, especially for nonlinear sensors
		case SENSOR_TEST0:
		{
			int sensor = open("/sys/devices/platform/tsc/ain0", O_RDONLY); //need unique path for each sensor ADC, check path in documentation
			char buffer[128]; 											//ADCs on Beaglebone are 12 bits?
			int read = read(sensor, buffer, sizeof(buffer);
			if (read != -1) {
				buffer[read] = NULL;										//string returned by read is not null terminated
				int value = atoi(buffer);
				double convert = (value/4096) * 1800;						//sample conversion from ADC input to 'true value'
                d->value = convert;
				lseek(sensor, 0, 0);										//after read string must be rewound to start of file using lseek
            }
			else {
				perror("Failed to get value from sensor");
			}
			close(sensor);
			break;
		}
		case SENSOR_TEST1:
			int sensor = open("/sys/devices/platform/tsc/ain1", O_RDONLY); 
			char buffer[128]; 	
			int read = read(sensor, buffer, sizeof(buffer);
			if (read != -1) {
				buffer[read] = NULL;	
				int value = atoi(buffer);
				double convert = (value/4096) * 1800;	
                d->value = convert;
				lseek(sensor, 0, 0);	
            }
			else {
				perror("Failed to get value from sensor");
			}
			close(sensor);
			break;
			break;
		}
		//TODO: think about a better way to address individual pins
		//		i.e. pass an int to a separate function that builds the correct filename
		//		doesn't really matter if the pins we're using are fixed though
		case DIGITAL_TEST0:
			int fd = open("/sys/class/gpio/gpio17/value", O_RDONLY)		//again need the right addresses for each pin
			char ch;													//just one character for binary info
			lseek(fd, 0, SEEK_SET);
			int read = read(fd, &ch, sizeof(ch);
			if (read != -1) {
				if (ch != '0') {
					d->value = 1;
				}
				else {
					d->value = 0;
				}
			}
			else {
				perror("Failed to get value from pin");	
			}
			break;
		case DIGITAL_TEST1:
			int fd = open("/sys/class/gpio/gpio23/value", O_RDONLY)
			char ch;
			lseek(fd, 0, SEEK_SET);
			int read = read(fd, &ch, sizeof(ch);
			if (read != -1) {
				if (ch != '0') {
					d->value = 1;
				}
				else {
					d->value = 0;
				}
			}
			else {
				perror("Failed to get value from pin");	
			}
			break;
		default:
			Fatal("Unknown sensor id: %d", sensor_id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling

	return d;
}