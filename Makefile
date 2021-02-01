# Makefile for the ChatKey program. Typing 'make' or 'make ck' will create the executable file.
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
default: ck

# General target format
# target: list of prerequisites

# To create the executable file ChatKey we need all of the object files.
# This is equivalent to the commented-out ck target below.
# ck: $(OBJ)
#	$(CC) $(CFLAGS) -o ChatKey $(OBJ)

# To create the executable file ChatKey we need the object files:
# server.o
ck: server.o
	$(CC) $(CFLAGS) -o ChatKey server.o

# To create the object file %.o, we need the source files:
# %.c (where % is a wildcard)
# $@: name of the target
# $<: name of the first prerequsite file
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html#Automatic-Variables
# %.o: %.c
#	$(CC) $(CFLAGS) $<

# To create the object file server.o, we need the source files:
# server.c (header files must be included in tag list, but not in gcc command)
server.o: server.c
	$(CC) $(CFLAGS) -c server.c

# To start over from scratch, type 'make clean'. This removes the executable file, as well as old
# .o object files.
clean: 
	rm *.exe *.o
