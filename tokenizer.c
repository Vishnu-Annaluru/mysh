#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
        char *tokenCopy = malloc(strlen(token) + 1);
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