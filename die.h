#ifndef DIE_H
#define DIE_H

/**
 * Prints the provided arguments separated by spaces and exits with failure status.
 * Any arguments to die should be printed, separated by spaces.
 *
 * @param args Array of string arguments to print
 * @param count Number of arguments
 */
void die(char **args, int count);

#endif /* DIE_H */