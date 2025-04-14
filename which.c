#include "which.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char* which(char* command){

    // Check if command is a built-in
    if( strcmp(command, "cd") == 0 || 
        strcmp(command, "pwd") == 0 || 
        strcmp(command, "which") == 0 ||
        strcmp(command, "exit") == 0 ||
        strcmp(command, "die") == 0
      ){
        return NULL;
    }
    
    // Searching paths in this order /usr/local/bin, /usr/bin, /bin
    char* paths[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    
    int i;
    for(i = 0; i < 3; i++){

        int pathLength = strlen(paths[i]) + strlen(command) + 2; // for '/' and null terminator
        
        char* path = malloc(pathLength);
        if(!path){
            return NULL;
        }
        
        snprintf(path, pathLength, "%s/%s", paths[i], command);
        
        // Check if file exists using access()
        if(access(path, X_OK) == 0){
            return path;
        }
        
        free(path); // If found, need to free in mysh
    }
    
    return NULL;
}