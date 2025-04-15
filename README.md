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

FIX


All three of these commands will simply ignore standard input, but pwd, which, and exit have output that could be sent to a file or piped to another program.

Also will you print out some error message if a command does not exist
