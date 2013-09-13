# Makefile for server software
CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = -lfcgi -lssl -lcrypto -lpthread -lm
OBJ = log.o control.o data.o fastcgi.o main.o sensor.o
RM = rm -f

BIN = server
#BIN2 = stream

all : $(BIN) $(BIN2)

#stream : stream.c
#	$(CXX) -o stream stream.c -I/usr/include/opencv -I/usr/include/opencv2/highgui -L/usr/lib -lopencv_highgui -lopencv_core -lopencv_ml -lopencv_imgproc -lm


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


	
