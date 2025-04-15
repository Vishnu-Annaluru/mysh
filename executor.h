#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "arraylist.h"

/**
 * Execute a single command with arguments
 * @param list Command list containing command and arguments
 * @param index Pointer to current position in list
 * @param end_idx Last index to consider (exclusive)
 */
void executeCommand(arraylist_t *list, int *index, int end_idx);

/**
 * Execute commands in a pipeline
 * @param list Command list
 * @param start_idx Start index for first command
 * @param pipe_idx Index of pipe token
 * @param end_idx End index (exclusive)
 */
void executePipeline(arraylist_t *list, int start_idx, int pipe_idx, int end_idx);

/**
 * Process and execute all commands in list
 * @param list Command list to execute
 * @param interactive Flag indicating interactive mode
 */
void executeCommands(arraylist_t *list);

#endif // EXECUTOR_H