#include "pwd.h"
#include <unistd.h>
#include <stdlib.h>

#define MAX_PATH 1024

char* pwd(){

    char* cwd = malloc(MAX_PATH);
    if(!cwd){
        return NULL;
    }

    char* directory = getcwd(cwd, MAX_PATH);

    if(directory != NULL){
        return cwd;
    }
    else{
        return NULL;
    }

}
