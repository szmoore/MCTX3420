# Makefile for server software
CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = -lfcgi -lssl -lcrypto -lpthread
OBJ = log.o control.o sensor.o fastcgi.o thread.o main.o
RM = rm -f

BIN = server

all : $(BIN) stream

stream : stream.c
	$(CXX) -o stream stream.c -I/usr/include/opencv -I/usr/include/opencv2/highgui -L/usr/lib -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_imgproc -lm


$(BIN) : $(OBJ)
	$(CXX) $(FLAGS) -o $(BIN) $(OBJ) $(LIB)


%.o : %.c
	$(CXX) $(FLAGS) -c $<



clean :
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


	
