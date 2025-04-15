#include "../include/die.h"
#include <stdio.h>
#include <stdlib.h>

// Terminate with failure after printing any arguments
void die(char **args, int count){

    if(count > 0){
        int i;
        for(i = 0; i < count; i++){
            printf("%s", args[i]);
            if(i < count - 1){
                printf(" ");
            }
        }
        printf("\n");
    }

    exit(EXIT_FAILURE);
}
