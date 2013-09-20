#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdint.h>

#include "common.h"

#define exportPath 		"/sys/class/gpio/export"
#define unexportPath	"/sys/class/gpio/unexport"
#define valuePath	 	"/sys/class/gpio/gpio"
#define directionPath	"/sys/class/gpio/gpio"
#define ADCPath 		"/sys/devices/platform/tsc/ain"

void pinExport(int GPIOPin);
void pinDirection(int GPIOPin, int io);
void pinSet(double value, int GPIOPin);
void pinUnexport(int GPIOPin);
int pinRead(int GPIOPin);
int ADCRead(int adc_num);
