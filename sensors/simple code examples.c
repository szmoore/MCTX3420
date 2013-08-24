//Code to blink an LED - just to illustrate that it's pretty easy
//Only important thing is which name to address the LED by

#include <stdio.h>
#include <unistd.h>
 
using namespace std;
 
int main(int argc, char** argv) {
  FILE *LEDHandle = NULL;
  char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr0/brightness";
  printf("\nStarting LED blink program wooo!\n");
  while(1){
    if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL){
      fwrite("1", sizeof(char), 1, LEDHandle);
      fclose(LEDHandle);
    }
    sleep(1);
    if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL){
      fwrite("0", sizeof(char), 1, LEDHandle);
      fclose(LEDHandle);
    }
    sleep(1);
  }
  return 0;
}

//Sample code that should read a pressure sensor pin (conversion factors
//are just random numbers). Again pretty simple.

#include STUFF
 
double pressure(char *string) {
        int value = atoi(string);
        double millivolts = (value / 4096.0) * 1800; //convert input to volts
        double pressure = (millivolts - 500.0) / 10.0; //convert volts to pressure
        return pressure;
}
 
void main() {
        int fd = open("/sys/devices/platform/tsc/ain2", O_RDONLY); //open pin signal
        while (1) {
                char buffer[1024];
                int ret = read(fd, buffer, sizeof(buffer)); //get data
                if (ret != -1) {
                        buffer[ret] = NULL;
                        double kpa = pressure(buffer);
                        printf("digital value: %s  kilopascals: %f\n", buffer, kpa);
                        lseek(fd, 0, 0);
                }
                sleep(1);
        }
        close(fd);
}