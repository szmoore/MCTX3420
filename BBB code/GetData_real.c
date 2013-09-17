char adc_dir = "/sys/devices/platform/tsc/ain";
char pin_dir = "/sys/class/gpio/gpio";

/** Open an ADC and return the voltage value from it
*	@param adc_num - ADC number, ranges from 0 to 7 on the Beaglebone
	@return the converted voltage value if successful
*/

//TODO: create a function to lookup the ADC or pin number instead of manually
//		specifying it here (so we can keep all the numbers in one place)

int OpenAnalog(int adc_num)
{
	char adc_path[40];
	snprintf(adc_path, sizeof(adc_path), "%s%d", adc_dir, adc_num);		//construct ADC path
	int sensor = open(adc_path, O_RDONLY);
	char buffer[128];								//I think ADCs are only 12 bits (0-4096)
	int read = read(sensor, buffer, sizeof(buffer);	//buffer can probably be smaller
	if (read != -1) {
		buffer[read] = NULL;
		int value = atoi(buffer);
		double convert = (value/4096) * 1000;		//random conversion factor, will be different for each sensor
		//lseek(sensor, 0, 0);
		close(sensor);
		return convert;
	}
	else {
		perror("Failed to get value from sensor");
		close(sensor);
		return -1;
	}
}

/** Open a digital pin and return the value from it
*	@param pin_num - pin number, specified by electronics team
	@return 1 or 0 if reading was successful
*/

int OpenDigital(int pin_num)
{
	char pin_path[40];
	snprintf(pin_path, sizeof(pin_path), "%s%d%s", pin_dir, pin_num, "/value");	//construct pin path
	int pin = open(pin_path, O_RDONLY);
	char ch;
	lseek(fd, 0, SEEK_SET);
	int read = read(pin, &ch, sizeof(ch);
	if (read != -1) {
		if (ch != '0') {
			close(pin);
			return 1;
		}
		else {
			close(pin);
			return 0;
		}
	else {
		perror("Failed to get value from pin");
		close(pin);
		return -1;
	}
}

/**
 * Read a DataPoint from a Sensor; block until value is read
 * @param id - The ID of the sensor
 * @param d - DataPoint to set
 * @returns True if the DataPoint was different from the most recently recorded.
 */
bool Sensor_Read(Sensor * s, DataPoint * d)
{
	
	// Set time stamp
	struct timeval t;
	gettimeofday(&t, NULL);
	d->time_stamp = TIMEVAL_DIFF(t, g_options.start_time);

	// Read value based on Sensor Id
	switch (s->id)
	{
		case ANALOG_TEST0:
			d->value = OpenAnalog(0);	//ADC #0 on the Beaglebone
			break;
		case ANALOG_TEST1:
		{
			d->value = OpenAnalog(1);
			break;
		}
		case ANALOG_FAIL0:
			d->value = (double)(rand() % 6) * -( rand() % 2) / ( rand() % 100 + 1);
			//Gives a value between -5 and 5
			break;
		case DIGITAL_TEST0:
			d->value = openDigital(0);	//replace 0 with correct pin number
			break;
		case DIGITAL_TEST1:
			d->value = openDigital(1);	//replace 1 with correct pin number
			break;
		case DIGITAL_FAIL0:
			if( rand() % 100 > 98)
				d->value = 2;
			d->value = rand() % 2; 
			//Gives 0 or 1 or a 2 every 1/100 times
			break;
		default:
			Fatal("Unknown sensor id: %d", s->id);
			break;
	}	
	usleep(100000); // simulate delay in sensor polling

	// Perform sanity check based on Sensor's ID and the DataPoint
	Sensor_CheckData(s->id, d->value);

	// Update latest DataPoint if necessary
	bool result = (d->value != s->newest_data.value);
	if (result)
	{
		s->newest_data.time_stamp = d->time_stamp;
		s->newest_data.value = d->value;
	}
	return result;
}