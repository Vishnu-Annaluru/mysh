CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined

OBJS = mysh.o arraylist.o pwd.o cd.o which.o die.o tokenizer.o wildcard.o executor.o
HEADERS = arraylist.h pwd.h cd.h which.h die.h tokenizer.h wildcard.h executor.h

all: mysh

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

which.o: which.c which.h
	$(CC) $(CFLAGS) -c which.c

die.o: die.c die.h
	$(CC) $(CFLAGS) -c die.c

tokenizer.o: tokenizer.c tokenizer.h
	$(CC) $(CFLAGS) -c tokenizer.c

wildcard.o: wildcard.c wildcard.h
	$(CC) $(CFLAGS) -c wildcard.c

executor.o: executor.c executor.h
	$(CC) $(CFLAGS) -c executor.c

clean:
	rm -rf *.o mysh