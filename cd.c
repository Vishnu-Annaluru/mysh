#include "cd.h"
#include <unistd.h>
#include <stdio.h>

void cd(char* path) {
    int c = chdir(path);
    if(c != 0){
        perror("cd");
    }
}