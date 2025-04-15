#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "arraylist.h"

/**
 * Parses a command string into a list of tokens
 * @param str The input string to tokenize
 * @return An arraylist containing the tokens
 */
arraylist_t getCommandList(char *str);

#endif // TOKENIZER_H