#ifndef TOKEN_H
#define TOKEN_H

#define MAX_TOKEN_LEN 256
#define MAX_TOKENS 128

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_APPEND,
    TOKEN_BACKGROUND,
    TOKEN_END
} TokenType;

typedef struct
{
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

#endif
