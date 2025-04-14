CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined

all: mysh
$(OBJS): mysh.o arraylist.o pwd.o cd.o
$(HEADERS): arraylist.h pwd.h cd.h

mysh: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mysh

mysh.o: mysh.c $(HEADERS)
	$(CC) $(CFLAGS) -c mysh.c

arraylist.o: arraylist.c arraylist.h
	$(CC) $(CFLAGS) -c arraylist.c

pwd.o: pwd.c pwd.h
	$(CC) $(CFLAGS) -c pwd.c

cd.o: cd.c cd.h
	$(CC) $(CFLAGS) -c cd.c

clean:
	rm -rf *.o mysh