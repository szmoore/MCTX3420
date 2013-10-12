# Makefile for server software
CXX = gcc
FLAGS = -std=c99 -Wall -pedantic -g -I../
#-I/usr/include/opencv -I/usr/include/opencv2/highgui For OpenCV
LIB = -lpthread
OBJ = ledtest.o filetest.o
HEADERS = $(wildcard *.h)
RM = rm -f

all : $(OBJ)
	ar rvs actuators.a $(OBJ)


%.o : %.c
	$(CXX) $(FLAGS) -c $<

clean :
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ)
	$(RM) *.*~
	$(RM) *~


	
