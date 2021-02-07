# Makefile for the ChatKey program. Typing 'make' or 'make ckserver' will create the executable file.
# Note: 'make' was added as a Cmder alias for 'mingw32-make'

# Define some Makefile variables for the compiler and compiler flags. To use Makefile variables
# later in the Makefile, wrap them in $().
CC=gcc
CFLAGS=-g -Wall

# Sources files
SRC=server.c

# Object files (find all %.c patterns in SRC and replace with %.o where % is a wildcard)
OBJ=$(patsubst %.c, %.o, $(SRC))

# Typing 'make' will invoke the first target entry in the file (in this case the default target
# entry). yYou can name this target entry anything, but "default" or "all" are the most commonly
# used names by convention.
default: ckserver ckclient

# General target format
# target: list of prerequisites

# To create the executable file ChatKeyServer we need all of the object files.
# This is equivalent to the commented-out ckserver target below.
# ckserver: $(OBJ)
#	$(CC) $(CFLAGS) -o ChatKeyServer $(OBJ)

# To create the executable file ChatKeyServer we need the object files:
# server.o
ckserver: server.o
	$(CC) $(CFLAGS) -o ChatKeyServer server.o -lws2_32

# To create the object file %.o, we need the source files:
# %.c (where % is a wildcard)
# $@: name of the target
# $<: name of the first prerequsite file
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html#Automatic-Variables
# %.o: %.c
#	$(CC) $(CFLAGS) $<

# To create the object file server.o, we need the source files:
# server.c (header files must be included in tag list, but not in gcc command)
server.o: server.c server.h
	$(CC) $(CFLAGS) -c server.c

# To create the executable file ChatKeyClient we need the object files:
# client.o
ckclient: client.o
	$(CC) $(CFLAGS) -o ChatKeyClient client.o -lws2_32

# To create the object file client.o, we need the source files:
# client.c (header files must be included in tag list, but not in gcc command)
client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c

# To start over from scratch, type 'make clean'. This removes the executable file, as well as old
# .o object files.
clean: 
	rm *.exe *.o
