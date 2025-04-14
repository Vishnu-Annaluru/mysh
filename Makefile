CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined

all: mysh

mysh: mysh.o arraylist.o pwd.o
	$(CC) $(CFLAGS) mysh.o arraylist.o pwd.o -o mysh

mysh.o: mysh.c arraylist.h pwd.h
	$(CC) $(CFLAGS) -c mysh.c

arraylist.o: arraylist.c arraylist.h
	$(CC) $(CFLAGS) -c arraylist.c

pwd.o: pwd.c pwd.h
	$(CC) $(CFLAGS) -c pwd.c

clean:
	rm -rf *.o mysh