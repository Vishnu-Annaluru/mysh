#include "../include/wildcard.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void expandWildcard(char *pattern, arraylist_t *expanded){
    
    // MAX_PATH is defined in the header :)

    char *lastSegment = strrchr(pattern, '/');
    char path[MAX_PATH] = ".";
    char filePattern[MAX_PATH];
    
    if(lastSegment){

        strncpy(path, pattern, lastSegment - pattern);
        path[lastSegment - pattern] = '\0';
        strcpy(filePattern, lastSegment + 1);

    }
    else{
        strcpy(filePattern, pattern);
    }
    
    char *asterisk = strchr(filePattern, '*');
    if(!asterisk){
        
        char *temp = malloc(strlen(pattern) + 1);
        if(temp == NULL){
            perror("malloc failed");
            return;
        }

        strcpy(temp, pattern);
        al_append(expanded, temp);
        return;
    }
    
    char before[MAX_PATH];
    char after[MAX_PATH];
    
    strncpy(before, filePattern, asterisk - filePattern);
    before[asterisk - filePattern] = '\0';
    strcpy(after, asterisk + 1);
    
    DIR *dir = opendir(path);
    if(!dir){
        
        char *temp = malloc(strlen(pattern) + 1);
        if(temp == NULL){
            perror("malloc failed");
            return;
        }
        
        strcpy(temp, pattern);
        al_append(expanded, temp);
        return;
    }
    
    // dirent
    struct dirent *entry;
    bool found = false;
    
    while((entry = readdir(dir)) != NULL){
        
        // ignoring files that start with a period
        if((entry->d_name[0]) == '.' && (before[0] != '.')){
            continue;
        }
        
        size_t nameLength = strlen(entry->d_name);
        size_t beforeLength = strlen(before);
        size_t afterLen = strlen(after);

        if((strncmp(entry->d_name, before, beforeLength) == 0) && (afterLen == 0 || strcmp(entry->d_name + nameLength - afterLen, after) == 0) && (nameLength >= beforeLength + afterLen)){
            
            char fullPath[MAX_PATH];
            if(strcmp(path, ".") == 0){
                if(snprintf(fullPath, MAX_PATH, "%s", entry->d_name) >= MAX_PATH){
                    fullPath[MAX_PATH - 1] = '\0';
                }
            }
            else{
                if(snprintf(fullPath, MAX_PATH, "%s/%s", path, entry->d_name) >= MAX_PATH){
                    fullPath[MAX_PATH - 1] = '\0';
                }
            }
            

            // allocate full path str

            char *temp = malloc(strlen(fullPath) + 1);
            if(temp == NULL){
                perror("malloc failed");
                closedir(dir);
                return;
            }

            strcpy(temp, fullPath);
            al_append(expanded, temp);
            found = true;

        }
    }
    
    // close directory
    closedir(dir);
    
    // add original if no matches
    if(!found){
        char *temp = malloc(strlen(pattern) + 1);
        if(temp == NULL){
            perror("malloc failed");
            return;
        }

        strcpy(temp, pattern);
        al_append(expanded, temp);
    }
}
