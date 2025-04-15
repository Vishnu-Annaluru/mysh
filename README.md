## Vishnu Annaluru (vta11)
# mysh

### Modes

Using isatty(), will either run in
#### Interactive Mode
Prints welcome and goodbye message and a prompt before each command
#### Batch Mode
Prints nothing

Any child process that is launched will close terminal standard input

Need to have one loop for both modes

### Input
Using read() to parse input commands, will not run until the previous command has been executed
Takes up to one argument, input will either be from standard input or from a file

### Termination
mysh will terminate when exit or die is executed, or if the input stream ends

### Commands
For each command, determine the path to the executable, the list of arguments, and the files to use for input and output

#### Command parsing
A token is a sequence of non-whitespace characters, but '<', '>', and '|' are their own tokens (These will be separated by whitespace)
'#' Starts a comment that continues until a newline char, and comments are ignored

If a token contains '*', it describes a pattern of file path names, and there will be only one in the final path segment
Open the directory and add all files that match the pattern to the argument list, if none, add the original token to the argument list

'<' and '>' will occur after normal arguments and the first argument after one of these will not be added to the argument list and is instead a redirection file name

'|' denotes a pipeline, where both programs have their own argument list and file redirections.

'and' and 'or' are not added to the argument list, a command starting with 'and' is executed if the previous command succeeded, and commands starting with 'or' execute when the previous command failed (checking the exit status)
Conditionals are not allowed in the first command, and they apply to the entire command even if there is '|'. However, 'and' or 'or' after a '|' is not allowed

### Exit Status
A command that launches a program succeeds if and only if that program exits normally with the exit code 0.
 - Built-in commands succeed if and only if they receive the correct number of arguments and
 successfully perform their operation. (For example, cd would fail if chdir() reports an error.)
 - Pipelines succeed if the last program in the pipeline succeeds.
 - When a conditional executes, it succeeds if the command succeeded. If the conditional does
   not execute, it is ignored.
 - A command containing a syntax error, such as the sequence < <, will fail.

mysh continues executing after a failed command. In the case of a syntax error, it will
skip ahead to the next newline and proceed from there.

mysh itself should always exit with status EXIT_SUCCESS, except after executing the command die or if it is unable to open its argument

### Programs
If the first argument to mysh c ontains a '/', it will be treated as an executable path

If it does not have a '/', and is not a built in program, look through '/usr/local/bin', then '/usr/bin' then '/bin'
Can use access()

#### Built-in programs
Implemented by mysh and are allowed to be used for redirection and pipes
- cd: Takes one argument and changes the working directory using chdir(), perror and fail if incorrect number of arguments or if chdir() fails
- pwd: Prints working directory to standard output using getcwd()
- which: Takes one argument and prings the path that mysh will use to exxeute the program, prints nothing if the argument is the name of a built-in, if incorrect number of
  arguments, or if the program is not found
- exit: Stop reading commands and terminate with EXIT_SUCCESS after executing any job involving exit
- die: Stop reading commands and terminate with EXIT_FAILURE, takes any number of arguments and prints each argument separated by spaces


All three of these commands will simply ignore standard input, but pwd, which, and exit have output that could be sent to a file or piped to another program.

Also will you print out some error message if a command does not exist


# DELETE THIS LATER PLEASE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "arraylist.h"

int strptrcmp(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}


int main(int argc, char **argv)
{
    arraylist_t a;
    al_init(&a, 1);

    for (int i = 0; i < argc; i++) {
	    al_append(&a, argv[i]);
    }

    al_append(&a, "secret extra argument");

    qsort(a.data, a.length, sizeof(char *), strptrcmp);

    al_append(&a, "extra appended after sorting");

    for (int i = 0; i < a.length; i++) {
	    printf("a[%d] = \"%s\"\n", i, a.data[i]);
    }

    al_destroy(&a);

    return EXIT_SUCCESS;
}



P3/
├── src/                # Source files
│   ├── mysh.c
│   ├── pwd.c
│   ├── cd.c
│   ├── which.c
│   ├── arraylist.c
├── include/            # Header files
│   ├── pwd.h
│   ├── cd.h
│   ├── which.h
│   ├── arraylist.h
├── bin/                # Executables
│   └── mysh
├── obj/                # Compiled object files
│   ├── mysh.o
│   ├── pwd.o
│   ├── cd.o
│   ├── which.o
│   ├── arraylist.o
├── Makefile
└── README.md




CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -Werror -fsanitize=address,undefined
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

OBJS = $(OBJ_DIR)/mysh.o $(OBJ_DIR)/arraylist.o $(OBJ_DIR)/pwd.o $(OBJ_DIR)/cd.o $(OBJ_DIR)/which.o
HEADERS = $(INC_DIR)/arraylist.h $(INC_DIR)/pwd.h $(INC_DIR)/cd.h $(INC_DIR)/which.h

all: $(BIN_DIR)/mysh

$(BIN_DIR)/mysh: $(OBJS)
    mkdir -p $(BIN_DIR)
    $(CC) $(CFLAGS) $(OBJS) -o $(BIN_DIR)/mysh

$(OBJ_DIR)/mysh.o: $(SRC_DIR)/mysh.c $(HEADERS)
    mkdir -p $(OBJ_DIR)
    $(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/mysh.c -o $(OBJ_DIR)/mysh.o

$(OBJ_DIR)/arraylist.o: $(SRC_DIR)/arraylist.c $(INC_DIR)/arraylist.h
    mkdir -p $(OBJ_DIR)
    $(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/arraylist.c -o $(OBJ_DIR)/arraylist.o

$(OBJ_DIR)/pwd.o: $(SRC_DIR)/pwd.c $(INC_DIR)/pwd.h
    mkdir -p $(OBJ_DIR)
    $(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/pwd.c -o $(OBJ_DIR)/pwd.o

$(OBJ_DIR)/cd.o: $(SRC_DIR)/cd.c $(INC_DIR)/cd.h
    mkdir -p $(OBJ_DIR)
    $(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/cd.c -o $(OBJ_DIR)/cd.o

$(OBJ_DIR)/which.o: $(SRC_DIR)/which.c $(INC_DIR)/which.h
    mkdir -p $(OBJ_DIR)
    $(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/which.c -o $(OBJ_DIR)/which.o

clean:
    rm -rf $(OBJ_DIR) $(BIN_DIR)