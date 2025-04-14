#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "arraylist.h"

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
        
        // Echo the command
        printf("You typed: %s\n", buffer);
        
        arraylist_t list = getCommandList(buffer);

        for (int i = 0; i < list.length; i++) {
            printf("Command %d = \"%s\"\n", i+1, list.data[i]);
        }

        al_destroy(&list);
        
        // Clean up
        free(buffer);
    }
    
    return EXIT_SUCCESS;
}