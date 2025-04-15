#include "executor.h"
#include "pwd.h"
#include "cd.h"
#include "which.h"
#include "wildcard.h"
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
    
    // Handle built-in commands (cd, which, pwd)
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
        
        // Parse arguments and handle redirection
        char *infile = NULL;
        char *outfile = NULL;
        int arg_count = 0;
        int i = *index;
        
        // First pass: count actual arguments and find redirection
        while (i < end_idx) {
            if (strcmp(list->data[i], "<") == 0 && i + 1 < end_idx) {
                infile = list->data[i + 1];
                i += 2; // Skip redirection token and filename
            } else if (strcmp(list->data[i], ">") == 0 && i + 1 < end_idx) {
                outfile = list->data[i + 1];
                i += 2; // Skip redirection token and filename
            } else {
                arg_count++;
                i++;
            }
        }
        
        // Allocate argument array
        char **args = malloc((arg_count + 2) * sizeof(char *));
        if (args == NULL) {
            free(path);
            perror("Memory allocation failed");
            return;
        }
        
        // Second pass: fill argument array (skipping redirection)
        args[0] = command;
        int arg_idx = 1;
        i = *index;
        
        while (i < end_idx && arg_idx <= arg_count) {
            if ((strcmp(list->data[i], "<") == 0 || 
                 strcmp(list->data[i], ">") == 0) && i + 1 < end_idx) {
                i += 2; // Skip redirection token and filename
            } else {
                args[arg_idx++] = list->data[i++];
            }
        }
        args[arg_idx] = NULL;
        
        // Fork and execute
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            free(path);
            free(args);
            return;
        } else if (pid == 0) {
            // Child process
            
            // Set up input redirection
            if (infile != NULL) {
                int fd = open(infile, O_RDONLY);
                if (fd == -1) {
                    perror("Failed to open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            // Set up output redirection
            if (outfile != NULL) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd == -1) {
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            execv(path, args);
            
            // If execv returns, there was an error
            perror("execv failed");
            free(path);
            free(args);
            exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);
            
            // Update index to skip all processed arguments
            *index = end_idx; // All arguments have been processed
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
        close(pipefd[0]); close(pipefd[1]);
        return;
    } else if (pid1 == 0) {
        // First command: stdout -> pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        int idx = start_idx;
        executeCommand(list, &idx, pipe_idx);
        
        // Important: Child process must exit after command execution
        // This prevents memory leaks by ensuring we don't return to the main loop
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        close(pipefd[0]); close(pipefd[1]);
        return;
    } else if (pid2 == 0) {
        // Second command: stdin <- pipe
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        int idx = pipe_idx + 1;
        executeCommand(list, &idx, end_idx);
        
        // Important: Child process must exit after command execution
        exit(EXIT_SUCCESS);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Helper function to duplicate a string without using strdup
char* duplicate_string(const char* str) {
    size_t len = strlen(str) + 1; // +1 for null terminator
    char* new_str = malloc(len);
    if (new_str == NULL) return NULL;
    strcpy(new_str, str);
    return new_str;
}

// Revised function to process wildcards in token list
void processWildcards(arraylist_t *list) {
    arraylist_t new_list;
    al_init(&new_list, list->length);

    for (int i = 0; i < list->length; i++) {
        char *token = list->data[i];

        // Don't expand wildcards in special tokens like |, <, >
        if (strcmp(token, "|") == 0 ||
            strcmp(token, "<") == 0 ||
            strcmp(token, ">") == 0) {
            // Always duplicate, then free the original
            char *new_token = duplicate_string(token);
            if (new_token) {
                al_append(&new_list, new_token);
            }
            free(token);
            continue;
        }

        // If token contains a wildcard, expand it
        if (strchr(token, '*') != NULL) {
            arraylist_t expanded;
            al_init(&expanded, 10);

            expandWildcard(token, &expanded);

            if (expanded.length > 0) {
                for (int j = 0; j < expanded.length; j++) {
                    al_append(&new_list, expanded.data[j]);
                }
            } else {
                // No matches, duplicate the original token
                char *new_token = duplicate_string(token);
                if (new_token) {
                    al_append(&new_list, new_token);
                }
            }
            free(expanded.data);
            free(token); // Always free the original after expansion
        } else {
            // No wildcard, duplicate and free the original
            char *new_token = duplicate_string(token);
            if (new_token) {
                al_append(&new_list, new_token);
            }
            free(token);
        }
    }

    free(list->data);
    list->data = new_list.data;
    list->length = new_list.length;
    list->capacity = new_list.capacity;
}

// Modify executeCommands to process wildcards first
void executeCommands(arraylist_t *list) {
    // Process wildcards before executing commands
    processWildcards(list);
    
    // Rest of the function remains the same
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