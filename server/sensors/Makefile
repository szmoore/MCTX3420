# Makefile for server software
CXX = gcc
FLAGS = -std=c99 -Wall -pedantic -g -I../ -I/usr/include/opencv -I/usr/include/opencv2/highgui #For OpenCV
LIB = -lpthread
OBJ = strain.o resource.o pressure.o dilatometer.o
HEADERS = $(wildcard *.h)
RM = rm -f

all.o : $(OBJ)
	ar rvs sensors.a $(OBJ)


%.o : %.c
	$(CXX) $(FLAGS) -c $<

clean :
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ)
	$(RM) *.*~
	$(RM) *~


	
