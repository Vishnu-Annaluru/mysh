CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined

all: mysh

mysh: mysh.o arraylist.o
	$(CC) $(CFLAGS) mysh.o arraylist.o -o mysh

mysh.o: mysh.c arraylist.h
	$(CC) $(CFLAGS) -c mysh.c

arraylist.o: arraylist.c arraylist.h
	$(CC) $(CFLAGS) -c arraylist.c

clean:
	rm -rf *.o mysh