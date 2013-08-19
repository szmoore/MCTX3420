#define _BSD_SOURCE
#include "common.h"
#include "fastcgi.h"
#include <time.h>

typedef struct Data {
	pthread_t sensors;
	pthread_mutex_t mutex;
	volatile int sensor_value;
} Data;

void Handler_Sensors(void *arg, char *params) 
{
	const char *key, *value;
	char buf[5];
	Data *data = (Data*) arg;
	
	//Begin a request only when you know the final result
	//E.g whether OK or not.
	FCGI_BeginJSON(STATUS_OK, "sensors");
   	while ((params = FCGI_KeyPair(params, &key, &value))) {
   		FCGI_BuildJSON(key, value);
   	}
   	pthread_mutex_lock(&(data->mutex));
   	snprintf(buf, 128, "%d", data->sensor_value);
   	FCGI_BuildJSON("sensor_value", buf); 
   	pthread_mutex_unlock(&(data->mutex));
   	FCGI_EndJSON();
}

void *SensorsLoop(void *arg) {
	Data *data = (Data*) arg;
	srand(time(NULL));
	//Csection
	while(1) {
		pthread_mutex_lock(&(data->mutex));
		data->sensor_value = rand() % 1024;
		pthread_mutex_unlock(&(data->mutex));
		usleep(200*1000); //200ms
	}
}

int main(int argc, char *argv[]) {
	Data data = {0};
	pthread_mutex_init(&(data.mutex), NULL);
	//data.mutex = PTHREAD_MUTEX_INITIALIZER;
	
//	pthread_create(&data.sensors, NULL, SensorsLoop, &data);
	FCGI_RequestLoop(&data);
}
