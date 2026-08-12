#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#define MAX_INPUT_LENGTH 1024
#define MAX_TOKENS 100

int tokenize(const char *input, Token tokens[]);

#endif
