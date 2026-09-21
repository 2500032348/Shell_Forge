#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 128
#define MAX_COMMANDS 32

typedef struct
{
    char *argv[MAX_ARGS];
    int argc;

    char *input;
    char *output;

    int append;
    int background;
} Command;

typedef struct
{
    Command commands[MAX_COMMANDS];
    int command_count;
} Pipeline;

int parse_pipeline(Token tokens[],
                   int token_count,
                   Pipeline *pipeline);

void print_pipeline(Pipeline *pipeline);

void free_pipeline(Pipeline *pipeline);

#endif
