# Makefile for web2io testing stuff
CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = 
OBJ = network.o log.o webserver.o
RM = rm -f

BIN = webserver

$(BIN) : $(OBJ)
	$(CXX) $(FLAGS) -o $(BIN) $(OBJ)


%.o : %.c
	$(CXX) $(FLAGS) -c $<



clean :
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


	
