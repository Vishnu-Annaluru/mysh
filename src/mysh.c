#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include "../include/arraylist.h"
#include "../include/tokenizer.h"
#include "../include/wildcard.h"
#include "../include/executor.h"

#define MAX_PATH 4096

// fix implementatoin
char* readLine(){

    char *buffer = malloc(256);
    if(!buffer){
        perror("malloc failed");
        return NULL;
    }
    
    int c = 0;
    ssize_t bytes;
    memset(buffer, 0, 256);
    
    while((bytes = read(STDIN_FILENO, &buffer[c], 1)) > 0){
        if(buffer[c] == '\n'){
            buffer[c] = '\0';
            break;
        }
        
        c++;
        if(c >= 255){
            break;
        }
    }
    
    if(bytes <= 0 && c == 0){
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

int main(int argc, char *argv[]){
    
    if(argc > 2){
        fprintf(stderr, "too many arguments\n");
        return EXIT_FAILURE;
    }
    
    // Batch mode
    if(argc == 2){
        
        int input_fd = open(argv[1], O_RDONLY);
        if(input_fd == -1){
            perror("failed to open input file");
            return EXIT_FAILURE;
        }
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    
    int interactive = isatty(STDIN_FILENO);
    bool active = true;
    
    if(interactive){
        printf("Welcome to mysh!!!\n");
    }
    
    while(active){

        if(interactive){
            printf("mysh> ");
            fflush(stdout);
        }
        
        char *buffer = readLine();
        if(buffer == NULL){
            active = false;
            break;
        }
        
        if(strlen(buffer) == 0){
            free(buffer);
            continue;
        }
        
        if(strcmp(buffer, "exit") == 0){
            if(interactive){ //
                printf("exiting\n");
            }
            free(buffer);
            active = false;
            break;
        }
        
        arraylist_t list = getCommandList(buffer);
        executeCommands(&list);
        
        int i;
        for(i = 0; i < list.length; i++){
            free(list.data[i]);
        }
        
        al_destroy(&list);
        free(buffer);
    }
    
    
    return EXIT_SUCCESS;
}