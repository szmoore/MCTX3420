#include "gpio.h"

void pinExport(int GPIOPin) {
	FILE *myOutputHandle = NULL;
	char GPIOString[4];
	char setValue[4];
	sprintf(GPIOString, "%d", GPIOPin);
	if ((myOutputHandle = fopen(exportPath, "ab")) == NULL){
		Log(LOGERR, "Unable to export GPIO pin %f\n", GPIOPin);
	}
	strcpy(setValue, GPIOString);
	fwrite(&setValue, sizeof(char), 2, myOutputHandle);
	fclose(myOutputHandle);
}

void pinDirection(int GPIOPin, int io) {
	char setValue[4];
	char GPIODirection[64];
	FILE *myOutputHandle = NULL;
	snprintf(GPIODirection, sizeof(GPIODirection), "%s%d%s", directionPath, GPIOPin, "/direction");
	if ((myOutputHandle = fopen(GPIODirection, "rb+")) == NULL){
		Log(LOGERR, "Unable to open direction handle for pin %f\n", GPIOPin);
	}
	if (io == 1) {
		strcpy(setValue,"out");
		fwrite(&setValue, sizeof(char), 3, myOutputHandle);
	else if (io == 0) {
		strcpy(setValue,"in");
		fwrite(&setValue, sizeof(char), 2, myOutputHandle);
	else Log(LOGERR, "GPIO direction must be 1 or 0\n");
	fclose(myOutputHandle);
}

void pinSet(double value, int GPIOPin) {
	int val = (int)value;
	char GPIOValue[64];
	char setValue[4];
	FILE *myOutputHandle = NULL;
	snprintf(GPIOValue, sizeof(GPIOValue), "%s%d%s", valuePath, GPIOPin, "/value");
	if (val == 1) {
		if ((myOutputHandle = fopen(GPIOValue, "rb+")) == NULL){
			Log(LOGERR, "Unable to open value handle for pin %f\n", GPIOPin);
		}
		strcpy(setValue, "1"); // Set value high
		fwrite(&setValue, sizeof(char), 1, myOutputHandle);
	}
	else if (val == 0){
		if ((myOutputHandle = fopen(GPIOValue, "rb+")) == NULL){
			Log(LOGERR, "Unable to open value handle for pin %f\n", GPIOPin);
		}
		strcpy(setValue, "0"); // Set value low
		fwrite(&setValue, sizeof(char), 1, myOutputHandle);
	}
	else Log(LOGERR, "GPIO value must be 1 or 0\n");
	fclose(myOutputHandle);
}

/** Open an ADC and return the voltage value from it
*	@param adc_num - ADC number, ranges from 0 to 7 on the Beaglebone
	@return the converted voltage value if successful
*/

//TODO: create a function to lookup the ADC or pin number instead of manually
//		specifying it here (so we can keep all the numbers in one place)

int ADCRead(int adc_num)
{
	char adc_path[64];
	snprintf(adc_path, sizeof(adc_path), "%s%d", ADCPath, adc_num);		// Construct ADC path
	int sensor = open(adc_path, O_RDONLY);								
	char buffer[64];													// I think ADCs are only 12 bits (0-4096), buffer can probably be smaller
	int read = read(sensor, buffer, sizeof(buffer);
	if (read != -1) {
		buffer[read] = NULL;
		int value = atoi(buffer);
		double convert = (value/4096) * 1000;							// Random conversion factor, will be different for each sensor (get from datasheets)
		// lseek(sensor, 0, 0); (I think this is uneeded as we are reopening the file on each sensor read; if sensor is read continously we'll need this
		close(sensor);
		return convert;
	}
	else {
		Log(LOGERR, "Failed to get value from ADC %f\n", adc_num);
		close(sensor);
		return -1;
	}
}

/** Open a digital pin and return the value from it
*	@param pin_num - pin number, specified by electronics team
	@return 1 or 0 if reading was successful
*/

int pinRead(int GPIOPin)
{
	char GPIOValue[64];
	snprintf(GPIOValue, sizeof(GPIOValue), "%s%d%s", valuePath, GPIOPin, "/value");	//construct pin path
	int pin = open(GPIOValue, O_RDONLY);
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
		Log(LOGERR, "Failed to get value from pin %f\n", GPIOPin);
		close(pin);
		return -1;
	}
}

void pinUnexport(int GPIOPin) {
	char setValue[4];
	char GPIOString[4];
	sprintf(GPIOString, "%d", GPIOPin);
	FILE *myOutputHandle = NULL;
	if ((myOutputHandle = fopen(unexportPath, "ab")) == NULL) {
		Log(LOGERR, "Couldn't unexport GPIO pin %f\n", GPIOPin);
	}
	strcpy(setValue, GPIOString);
	fwrite(&setValue, sizeof(char), 2, myOutputHandle);
	fclose(myOutputHandle);
}