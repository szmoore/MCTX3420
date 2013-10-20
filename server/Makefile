# Makefile for server software
CXX = gcc
FLAGS = -std=gnu99 -Wall -pedantic -g -I/usr/include/opencv -I/usr/include/opencv2/highgui -L/usr/lib `mysql_config --cflags`
LIB = -lfcgi -lssl -lcrypto -lpthread -lm -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_imgproc -lldap -lcrypt `mysql_config --libs`
OBJ = log.o control.o data.o fastcgi.o main.o sensor.o actuator.o image.o bbb_pin.o pin_test.o login.o sensors/sensors.a actuators/actuators.a
RM = rm -f

BIN = server
#BIN2 = stream

all :
	$(MAKE) -C sensors
	$(MAKE) -C actuators
	$(MAKE) $(BIN)



$(BIN) : $(OBJ)
	$(CXX) $(FLAGS) -o $(BIN) $(OBJ) $(LIB)


%.o : %.c
	$(CXX) $(FLAGS) -c $<


clean :
	make -C sensors clean
	$(RM) $(BIN) $(BIN2)
	$(RM) *.o

clean_full: #cleans up all backup files
	make -C sensors clean_full
	$(RM) $(BIN) $(BIN2) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


	
