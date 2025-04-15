#include "die.h"
#include <stdio.h>
#include <stdlib.h>

void die(char **args, int count) {
    // Print arguments separated by spaces
    for (int i = 0; i < count; i++) {
        printf("%s", args[i]);
        if (i < count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    
    // Exit with failure status
    exit(EXIT_FAILURE);
}