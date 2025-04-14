#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include "arraylist.h"
#include "pwd.h"
#include "cd.h"
#include "which.h"

#define MAX_PATH 4096

// Function prototypes to avoid implicit declarations
arraylist_t getCommandList(char *str);
void expandWildcard(char *pattern, arraylist_t *expanded);
void executeCommand(arraylist_t *list, int *index, int end_idx);
void executePipeline(arraylist_t *list, int start_idx, int pipe_idx, int end_idx);
void executeCommands(arraylist_t *list);
char* readLine(void);

arraylist_t getCommandList(char *str) {
    arraylist_t list;
    al_init(&list, 10);
    
    // Remove comments: ignore everything after '#'
    char *comment = strchr(str, '#');
    if (comment != NULL) {
        *comment = '\0';
    }
    
    char *token = strtok(str, " \t\n");
    while (token != NULL) {
        char *tokenCopy = malloc(strlen(token) + 1);  // replacement for strdup(token)
        if (tokenCopy == NULL) {
            perror("malloc failed");
            return list;
        }
        strcpy(tokenCopy, token);
        al_append(&list, tokenCopy);
        token = strtok(NULL, " \t\n");
    }
    
    return list;
}

void expandWildcard(char *pattern, arraylist_t *expanded) {
    // Find the directory and filename pattern
    char *lastSlash = strrchr(pattern, '/');
    char dir_path[MAX_PATH] = ".";
    char file_pattern[MAX_PATH];
    
    if (lastSlash) {
        strncpy(dir_path, pattern, lastSlash - pattern);
        dir_path[lastSlash - pattern] = '\0';
        strcpy(file_pattern, lastSlash + 1);
    } else {
        strcpy(file_pattern, pattern);
    }
    
    // Find the prefix and suffix around the asterisk
    char *asterisk = strchr(file_pattern, '*');
    if (!asterisk) {
        char *copy = malloc(strlen(pattern) + 1);
        if (copy == NULL) {
            perror("malloc failed");
            return;
        }
        strcpy(copy, pattern);
        al_append(expanded, copy);
        return;
    }
    
    char prefix[MAX_PATH];
    char suffix[MAX_PATH];
    
    strncpy(prefix, file_pattern, asterisk - file_pattern);
    prefix[asterisk - file_pattern] = '\0';
    strcpy(suffix, asterisk + 1);
    
    DIR *dir = opendir(dir_path);
    if (!dir) {
        char *copy = malloc(strlen(pattern) + 1);
        if (copy == NULL) {
            perror("malloc failed");
            return;
        }
        strcpy(copy, pattern);
        al_append(expanded, copy);
        return;
    }
    
    struct dirent *entry;
    bool found_match = false;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && prefix[0] != '.')
            continue;
        
        size_t name_len = strlen(entry->d_name);
        size_t prefix_len = strlen(prefix);
        size_t suffix_len = strlen(suffix);
        if (name_len >= prefix_len + suffix_len &&
            strncmp(entry->d_name, prefix, prefix_len) == 0 &&
            (suffix_len == 0 || strcmp(entry->d_name + name_len - suffix_len, suffix) == 0)) {
            
            char full_path[MAX_PATH];
            if (strcmp(dir_path, ".") == 0) {
                if (snprintf(full_path, MAX_PATH, "%s", entry->d_name) >= MAX_PATH)
                    full_path[MAX_PATH - 1] = '\0';
            } else {
                if (snprintf(full_path, MAX_PATH, "%s/%s", dir_path, entry->d_name) >= MAX_PATH)
                    full_path[MAX_PATH - 1] = '\0';
            }
            
            char *copy = malloc(strlen(full_path) + 1);
            if (copy == NULL) {
                perror("malloc failed");
                closedir(dir);
                return;
            }
            strcpy(copy, full_path);
            al_append(expanded, copy);
            found_match = true;
        }
    }
    
    closedir(dir);
    
    if (!found_match) {
        char *copy = malloc(strlen(pattern) + 1);
        if (copy == NULL) {
            perror("malloc failed");
            return;
        }
        strcpy(copy, pattern);
        al_append(expanded, copy);
    }
}

// Modify the executeCommand function to respect boundaries
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
            
            // Clean up all allocated memory in the child process
            for (int i = 0; i < list->length; i++) {
                free(list->data[i]); // Free all allocated command tokens
            }
            free(list->data);
            
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

// Update executePipeline to use the modified executeCommand
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

// Update executeCommands to use the modified executeCommand
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
    
    if (interactive)
        printf("Bye bye.\n");
    
    return EXIT_SUCCESS;
}