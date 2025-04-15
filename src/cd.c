#include "../include/cd.h"
#include <unistd.h>
#include <stdio.h>

// Change working directory using chdir()
void cd(char* path){
    int c = chdir(path);
    if(c != 0){
        perror("cd");
    }
}
