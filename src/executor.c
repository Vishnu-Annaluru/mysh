#include "../include/executor.h"
#include "../include/pwd.h"
#include "../include/cd.h"
#include "../include/which.h"
#include "../include/wildcard.h"
#include "../include/die.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

// Function to execute a command, start and end are the index of the command in the list
void executeCommand(arraylist_t *list, int *start, int end){
    
    // If no end provided (default value -1), use list length
    if(end == -1){
        end = list->length;
    }
    
    if(*start >= end){
        return;
    }
    
    char *command = list->data[*start];
    (*start)++;
    
    // Built-in commands
    if(strcmp(command, "cd") == 0){
        if(*start < end){
            char *path = list->data[*start];
            (*start)++;
            cd(path);
        }
        else{
            fprintf(stderr, "cd: wrong number of arguments\n");
        }
    }
    else if(strcmp(command, "which") == 0){
        
        if(*start < end){
            
            char *cmd = list->data[*start];
            char *path = which(cmd);
            if(path != NULL){
                printf("%s\n", path);
                fflush(stdout);
                free(path);
            }
            (*start)++;
        }
    }
    else if(strcmp(command, "pwd") == 0){
        
        char *directory = pwd();
        if(directory != NULL){
            printf("%s\n", directory);
            fflush(stdout);
            free(directory);
        }
        else{
            perror("pwd failed");
        }
    }
    else if(strcmp(command, "die") == 0){

        int argc = end - *start;

        char **args = malloc((argc + 1) * sizeof(char *));
        if(!args){
            perror("malloc failed");
            return;
        }
        
        for(int i = 0; i < argc; i++){
            args[i] = list->data[*start + i];
        }
        args[argc] = NULL;
        
        die(args, argc); // this will exit
        
        free(args);
    }
    else{ // Everything else, organize later
        
        char *path;
        if(strchr(command, '/') != NULL){
            
            path = malloc(strlen(command) + 1);
            if(path == NULL){
                perror("malloc failed");
                return;
            }
            
            strcpy(path, command);
        }
        else{
            path = which(command);
            if(path == NULL)
                return;
        }
        
        // Redirection
        
        char *infile = NULL;
        char *outfile = NULL;
        int argc = 0;
        int i = *start;
        
        while(i < end){

            if(strcmp(list->data[i], "<") == 0 && i + 1 < end){
                
                infile = list->data[i + 1];
                i += 2;
            }
            else if(strcmp(list->data[i], ">") == 0 && i + 1 < end){
                outfile = list->data[i + 1];
                i += 2;
            }
            else{
                argc++;
                i++;
            }
        }
        
        
        char **args = malloc((argc + 2) * sizeof(char *));
        if(args == NULL){
            free(path);
            perror("malloc failed");
            return;
        }
        



        args[0] = command;
        int index = 1;
        i = *start;
        
        while(i < end && index <= argc){
            if(((strcmp(list->data[i], "<") == 0) || (strcmp(list->data[i], ">") == 0)) && (i + 1 < end)){
                i += 2;
            }
            else{
                args[index++] = list->data[i++];
            }
        }

        args[index] = NULL;
        
        // Fork proceses and execute them

        pid_t pid = fork();
        if(pid < 0){
            perror("fork failed");
            free(path);
            free(args);
            return;
        }
        else if(pid == 0){
            
            // Child process
            
            // input
            if(infile != NULL){
                
                int fd = open(infile, O_RDONLY);
                if(fd == -1){
                    perror("failed to open input file");
                    exit(EXIT_FAILURE);
                }
                
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            // output
            if(outfile != NULL){
                
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if(fd == -1){
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            execv(path, args);
            
            perror("execv failed");
            free(path);
            free(args);
            exit(EXIT_FAILURE);

        }
        else{ // Parent process needs to wait 
            int status;
            waitpid(pid, &status, 0);
            *start = end;
        }
        
        free(path);
        free(args);
    }
}

void executePipeline(arraylist_t *list, int start_index, int pipe_idx, int end){
    
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("pipe failed");
        return;
    }

    // First
    pid_t pid1 = fork();
    if(pid1 < 0){
        perror("fork failed");
        close(pipefd[0]); close(pipefd[1]);
        return;
    }
    else if(pid1 == 0){
        
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        int index = start_index;
        executeCommand(list, &index, pipe_idx);
        
        // have to exit
        exit(EXIT_SUCCESS);
    }

    // Second
    pid_t pid2 = fork();
    if(pid2 < 0){
        perror("fork failed");
        close(pipefd[0]); close(pipefd[1]);
        return;
    }
    else if(pid2 == 0){
        
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        int commandIndex = pipe_idx + 1;
        executeCommand(list, &commandIndex, end);
        
        exit(EXIT_SUCCESS);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

}


static char* duplicateString(char* str){
    size_t len = strlen(str) + 1;
    
    char* temp = malloc(len);
    if(temp == NULL){
        perror("malloc failed");
        return NULL;
    }
    
    strcpy(temp, str);
    return temp;
}

static void executeWildcard(arraylist_t *list){

    arraylist_t newList;
    al_init(&newList, list->length);

    int i;
    for(i = 0; i < list->length; i++){
        char *token = list->data[i];

        if(strcmp(token, "|") == 0 || strcmp(token, "<") == 0 || strcmp(token, ">") == 0){
            
            // was getting memory leaks
            char *temp = duplicateString(token);
            if(temp){
                al_append(&newList, temp);
            }

            free(token);
            continue;

        }

        // 'expanding' wildcard glob guy
        if(strchr(token, '*') != NULL){
            
            arraylist_t expanded;
            al_init(&expanded, 10);

            expandWildcard(token, &expanded);
            

            if(expanded.length > 0){
                int j;
                for(j = 0; j < expanded.length; j++){
                    al_append(&newList, expanded.data[j]);
                }
            }
            else{ // Add original if there are no matches to the arugment list
                char *temp = duplicateString(token);
                if(temp){
                    al_append(&newList, temp);
                }
            }
            //al_destroy(&expanded);
            free(expanded.data);
            free(token);
        }
        else{

            char *temp = duplicateString(token);
            if(temp){
                al_append(&newList, temp);
            }
            free(token);
        }
    }

    free(list->data);
    list->data = newList.data;
    list->length = newList.length;
    list->capacity = newList.capacity;

}

void executeCommands(arraylist_t *list){
    
    // Wildcard first
    executeWildcard(list);
    int i;

    
    int index = -1;

    for(i = 0; i < list->length; i++){
        if(strcmp(list->data[i], "|") == 0){
            index = i;
            break;
        }
    }
    
    if(index >= 0){
        executePipeline(list, 0, index, list->length);
    }
    else{
        int index = 0;
        executeCommand(list, &index, -1);
    }
}
