# Makefile for server software
CXX = gcc
FLAGS = -std=c99 -Wall -pedantic -g -I/usr/include/opencv -I/usr/include/opencv2/highgui -L/usr/lib
LIB = -lfcgi -lssl -lcrypto -lpthread -lm -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_imgproc
OBJ = log.o control.o data.o fastcgi.o main.o sensor.o actuator.o image.o bbb_pin.o pin_test.o
RM = rm -f

BIN = server
#BIN2 = stream

all : $(BIN) $(BIN2)


$(BIN) : $(OBJ)
	$(CXX) $(FLAGS) -o $(BIN) $(OBJ) $(LIB)


%.o : %.c
	$(CXX) $(FLAGS) -c $<



clean :
	$(RM) $(BIN) $(BIN2)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(BIN2) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


	
