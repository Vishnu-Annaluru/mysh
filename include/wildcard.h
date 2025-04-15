#ifndef WILDCARD_H
#define WILDCARD_H

#include "arraylist.h"

#define MAX_PATH 4096

/**
 * Expands a wildcard pattern into matching file paths
 * @param pattern The pattern containing a wildcard (*)
 * @param expanded The arraylist to append results to
 */
void expandWildcard(char *pattern, arraylist_t *expanded);

#endif // WILDCARD_H