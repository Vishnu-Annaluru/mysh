#include "../include/wildcard.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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