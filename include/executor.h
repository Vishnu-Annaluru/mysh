#include "arraylist.h"

void executeCommand(arraylist_t *list, int *index, int end_idx);

void executePipeline(arraylist_t *list, int start_idx, int pipe_idx, int end_idx);

void executeCommands(arraylist_t *list);
