# Makefile for rpi side server
CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = -lpthread
OBJ = log.o sensor.o test_request.o main.o
RM = rm -f

BIN = server

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


	
