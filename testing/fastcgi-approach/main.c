#define _BSD_SOURCE
#include "fastcgi.h"
#include "common.h"
#include <time.h>

typedef struct Data {
	pthread_t sensors;
	pthread_mutex_t mutex;
	volatile int sensor_value;
} Data;

Data data;

void Handler_Sensors(FCGIContext *context, char *params) 
{
	const char *key, *value;

	//Begin a request only when you know the final result
	//E.g whether OK or not.
	FCGI_BeginJSON(context, STATUS_OK);
   	while ((params = FCGI_KeyPair(params, &key, &value))) {
   		FCGI_JSONPair(key, value);
   	}
   	pthread_mutex_lock(&(data.mutex));
   	FCGI_JSONLong("sensor_value", data.sensor_value);
   	pthread_mutex_unlock(&(data.mutex));
   	FCGI_EndJSON();
}

void *SensorsLoop(void *arg) {
	srand(time(NULL));
	//Csection
	while(1) {
		pthread_mutex_lock(&(data.mutex));
		data.sensor_value = rand() % 1024;
		pthread_mutex_unlock(&(data.mutex));
		usleep(200*1000); //200ms
	}
}

int main(int argc, char *argv[]) {
	pthread_mutex_init(&(data.mutex), NULL);
	//data.mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_create(&data.sensors, NULL, SensorsLoop, &data);
	FCGI_RequestLoop(&data);
}
