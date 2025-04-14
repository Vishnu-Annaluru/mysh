typedef struct {
    char **data;
    unsigned int capacity;
    unsigned int length;
} arraylist_t;

int al_init(arraylist_t *, unsigned int cap);
int al_destroy(arraylist_t *);
int al_clear(arraylist_t *);

int al_append(arraylist_t *, char *);
int al_remove(arraylist_t *, char **);
