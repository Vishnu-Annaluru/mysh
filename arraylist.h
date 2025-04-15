#ifndef ARRAYLIST_H
#define ARRAYLIST_H

typedef struct {
    char **data;
    int length;
    int capacity;
} arraylist_t;

int al_init(arraylist_t *, unsigned int cap);
int al_destroy(arraylist_t *);
int al_clear(arraylist_t *);

int al_append(arraylist_t *, char *);
int al_remove(arraylist_t *, char **);

#endif /* ARRAYLIST_H */
