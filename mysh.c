#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include "arraylist.h"
#include "pwd.h"
#include "cd.h"
#include "which.h"

arraylist_t getCommandList(char *str){
    arraylist_t list;
    al_init(&list, 10);
    
    char *token = strtok(str, " ");
    while (token != NULL) {
        al_append(&list, token);
        token = strtok(NULL, " ");
    }
    
    return list;
}

void executeCommand(arraylist_t *list, int *index) {
    char *command = list->data[*index];
    (*index)++;
    
    if(strcmp(command, "cd") == 0) {
        // Check if there's a path argument
        if (*index < list->length) {
            char *path = list->data[*index];
            (*index)++; // Skip the path argument
            cd(path);
        } else {
            fprintf(stderr, "cd: missing argument\n");
        }
    } else if(strcmp(command, "which") == 0) {
        if (*index < list->length) {
            char *cmd = list->data[*index];
            char* path = which(cmd);
            if (path != NULL) {
                printf("%s\n", path);
                fflush(stdout);
                free(path);
            } else {
                fprintf(stderr, "which: command not found\n");
            }
            (*index)++;
        } else {
            fprintf(stderr, "which: missing argument\n");
        }
    } else if(strcmp(command, "pwd") == 0) {
        char* directory = pwd();
        if(directory != NULL) {
            printf("%s\n", directory);
            fflush(stdout);
            free(directory);
        } else {
            perror("pwd failed");
        }
    }
}

void executeCommands(arraylist_t *list) {
    int i = 0;
    while (i < list->length) {
        executeCommand(list, &i);
    }
}

char* readLine() {
    char* buffer = malloc(256 * sizeof(char));
    if (!buffer) {
        perror("malloc failed");
        return NULL;
    }
    
    int pos = 0;
    ssize_t bytes;
    
    memset(buffer, 0, 256);
    
    // Read until newline or EOF
    while ((bytes = read(STDIN_FILENO, &buffer[pos], 1)) > 0) {
        if (buffer[pos] == '\n') {
            buffer[pos] = '\0'; // Replace newline with null terminator
            break;
        }
        pos++;
        // Prevent overflow
        if (pos >= 255) {
            break;
        }
    }
    
    // If EOF or error with no content read
    if (bytes <= 0 && pos == 0) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Too many arguments\n");
        return EXIT_FAILURE;
    }
    
    int interactive = isatty(STDIN_FILENO);
    bool active = true;
    
    // Welcome message
    if (interactive) {
        printf("Welcome to mysh!!!\n");
    }
    
    // Main shell loop
    while(active) {
        // Prompt
        if (interactive) {
            printf("mysh> ");
            fflush(stdout);
        }
        
        char* buffer = readLine();
        
        // If EOF or error
        if (buffer == NULL) {
            printf("\nEnd of input\n");
            break;
        }
        
        // If the user typed "exit", stop
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting\n");
            free(buffer);
            break;
        }

        
       
        
        arraylist_t list = getCommandList(buffer);
        // int i;
        // for (i = 0; i < list.length; i++) {
        //     printf("Command %d = \"%s\"\n", i+1, list.data[i]);
        // }
        
        executeCommands(&list);

        al_destroy(&list);
        
        // Clean up
        free(buffer);
    }
    
    return EXIT_SUCCESS;
}