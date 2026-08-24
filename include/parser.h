#ifndef PARSER_H
#define PARSER_H

#define MAX_TOKENS 128
#define MAX_TOKEN_LEN 256
#define MAX_ARGS 128
#define MAX_COMMANDS 32

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_APPEND,
    TOKEN_BACKGROUND,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

typedef struct {
    char *argv[MAX_ARGS];

    char *input;
    char *output;

    int append;
    int background;
} Command;

typedef struct {
    Command commands[MAX_COMMANDS];
    int command_count;
} Pipeline;

int tokenize(const char *line, Token tokens[]);
void print_tokens(Token tokens[], int count);

int parse_pipeline(Token tokens[], int token_count, Pipeline *pipeline);
void print_pipeline(Pipeline *pipeline);

void free_pipeline(Pipeline *pipeline);

#endif
