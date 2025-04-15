#include "executor.h"
#include "pwd.h"
#include "cd.h"
#include "which.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void executeCommand(arraylist_t *list, int *index, int end_idx) {
    // If no end_idx provided (default value -1), use list length
    if (end_idx == -1) {
        end_idx = list->length;
    }
    
    if (*index >= end_idx) {
        return;
    }
    
    char *command = list->data[*index];
    (*index)++;
    
    if (strcmp(command, "cd") == 0) {
        if (*index < end_idx) {
            char *path = list->data[*index];
            (*index)++;
            cd(path);
        } else {
            fprintf(stderr, "cd: missing argument\n");
        }
    } else if (strcmp(command, "which") == 0) {
        if (*index < end_idx) {
            char *cmd = list->data[*index];
            char *path = which(cmd);
            if (path != NULL) {
                printf("%s\n", path);
                fflush(stdout);
                free(path);
            }
            (*index)++;
        }
    } else if (strcmp(command, "pwd") == 0) {
        char *directory = pwd();
        if (directory != NULL) {
            printf("%s\n", directory);
            fflush(stdout);
            free(directory);
        } else {
            perror("pwd failed");
        }
    } else {
        // External command execution
        char *path;
        if (strchr(command, '/') != NULL) {
            path = malloc(strlen(command) + 1);
            if (path == NULL) {
                perror("Memory allocation failed");
                return;
            }
            strcpy(path, command);
        } else {
            path = which(command);
            if (path == NULL)
                return;
        }
        
        // Count remaining arguments for the command (up to end_idx)
        int arg_count = 0, i = *index;
        while (i < end_idx) {
            arg_count++;
            i++;
        }
        
        char **args = malloc((arg_count + 2) * sizeof(char *));
        if (args == NULL) {
            free(path);
            perror("Memory allocation failed");
            return;
        }
        
        args[0] = command;
        for (i = 0; i < arg_count; i++) {
            args[i + 1] = list->data[*index + i];
        }
        args[arg_count + 1] = NULL;
        
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            free(path);
            free(args);
            return;
        } else if (pid == 0) {
            // Child process
            execv(path, args);
            
            // If execv returns, there was an error
            perror("execv failed");
            free(path);
            free(args);
            // Clean up any allocated resources if needed before exiting
            exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);
            *index += arg_count;
        }
        
        free(path);
        free(args);
    }
}

void executePipeline(arraylist_t *list, int start_idx, int pipe_idx, int end_idx) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }
    
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    } else if (pid1 == 0) {
        // Child for first command
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        int idx = start_idx;
        executeCommand(list, &idx, pipe_idx);  // Stop at pipe
        
        // Free all memory in child process before exiting
        for (int i = 0; i < list->length; i++) {
            free(list->data[i]);  // Free each string
        }
        free(list->data);         // Free the array
        exit(EXIT_SUCCESS);
    }
    
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    } else if (pid2 == 0) {
        // Child for second command
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        int idx = pipe_idx + 1;
        executeCommand(list, &idx, end_idx);  // Stop at end
        
        // Free all memory in child process before exiting
        for (int i = 0; i < list->length; i++) {
            free(list->data[i]);  // Free each string
        }
        free(list->data);         // Free the array
        exit(EXIT_SUCCESS);
    }
    
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void executeCommands(arraylist_t *list) {
    int pipe_idx = -1;
    for (int i = 0; i < list->length; i++) {
        if (strcmp(list->data[i], "|") == 0) {
            pipe_idx = i;
            break;
        }
    }
    
    if (pipe_idx >= 0)
        executePipeline(list, 0, pipe_idx, list->length);
    else {
        int idx = 0;
        executeCommand(list, &idx, -1);  // -1 means use list->length
    }
}