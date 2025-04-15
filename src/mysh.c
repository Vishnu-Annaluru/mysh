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

char* readLine() {
    char *buffer = malloc(256);
    if (!buffer) {
        perror("malloc failed");
        return NULL;
    }
    
    int pos = 0;
    ssize_t bytes;
    memset(buffer, 0, 256);
    
    while ((bytes = read(STDIN_FILENO, &buffer[pos], 1)) > 0) {
        if (buffer[pos] == '\n') {
            buffer[pos] = '\0';
            break;
        }
        pos++;
        if (pos >= 255) {
            break;
        }
    }
    
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
    
    if (argc == 2) {
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("Failed to open input file");
            return EXIT_FAILURE;
        }
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    
    int interactive = isatty(STDIN_FILENO);
    bool active = true;
    
    if (interactive)
        printf("Welcome to mysh!!!\n");
    
    while (active) {
        if (interactive) {
            printf("mysh> ");
            fflush(stdout);
        }
        
        char *buffer = readLine();
        if (buffer == NULL) {
            active = false;
            break;
        }
        
        if (strlen(buffer) == 0) {
            free(buffer);
            continue;
        }
        
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "die") == 0) {
            if (interactive)
                printf("exiting\n");
            free(buffer);
            active = false;
            break;
        }
        
        arraylist_t list = getCommandList(buffer);
        executeCommands(&list);
        
        // Make sure every string in the list is freed
        for (int i = 0; i < list.length; i++) {
            free(list.data[i]);
        }
        
        // Then free the list data array itself
        al_destroy(&list);
        free(buffer);
    }
    
    
    
    return EXIT_SUCCESS;
}