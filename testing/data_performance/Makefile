# Makefile for rpi side server
CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = -lpthread -lsqlite3
BIN = binfile csv sqlite
RM = rm -f



all : $(BIN)

% : %.c
	$(CXX) $(FLAGS) -o $@ $< $(LIB)



clean :
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


	
