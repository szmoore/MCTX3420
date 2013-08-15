CXX = gcc
FLAGS = -std=c99 -Wall -Werror -pedantic -g
LIB = -lpthread -lfcgi
OBJ = fastcgi.o
RM = rm -f

BIN = fast

$(BIN) : $(OBJ)
	$(CXX) $(FLAGS) -o $(BIN) $(OBJ) $(LIB)

%.o : %.c
	$(CXX) $(FLAGS) -c $<

clean:
	$(RM) $(BIN)
	$(RM) *.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~
