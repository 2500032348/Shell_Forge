#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "token.h"

static int is_operator(char c)
{
    return (c == '+' || c == '-' ||
            c == '=' || c == '*' ||
            c == '%' || c == '&');
}

int tokenize(const char *input, Token tokens[])
{
    int i = 0;
    int token_count = 0;

    while (input[i] != '\0' &&
           token_count < MAX_TOKENS - 1)
    {
        /* Skip spaces */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (input[i] == '|')
        {
            tokens[token_count].type = TOKEN_PIPE;
            strcpy(tokens[token_count].value, "|");

            token_count++;
            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            tokens[token_count].type = TOKEN_REDIRECT_IN;
            strcpy(tokens[token_count].value, "<");

            token_count++;
            i++;
            continue;
        }

        /* Output redirection */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                tokens[token_count].type = TOKEN_REDIRECT_APPEND;
                strcpy(tokens[token_count].value, ">>");

                i += 2;
            }
            else
            {
                tokens[token_count].type = TOKEN_REDIRECT_OUT;
                strcpy(tokens[token_count].value, ">");

                i++;
            }

            token_count++;
            continue;
        }

        /* Semicolon */
        if (input[i] == ';')
        {
            tokens[token_count].type = TOKEN_SEMICOLON;
            strcpy(tokens[token_count].value, ";");

            token_count++;
            i++;
            continue;
        }

        /* String */
        if (input[i] == '"' || input[i] == '\'')
        {
            char quote = input[i];
            int j = 0;

            i++;

            while (input[i] != '\0' &&
                   input[i] != quote &&
                   j < MAX_TOKEN_LENGTH - 1)
            {
                tokens[token_count].value[j++] = input[i++];
            }

            tokens[token_count].value[j] = '\0';

            if (input[i] == quote)
                i++;

            tokens[token_count].type = TOKEN_STRING;

            token_count++;
            continue;
        }

        /* Number */
        if (isdigit((unsigned char)input[i]))
        {
            int j = 0;

            while (isdigit((unsigned char)input[i]) &&
                   j < MAX_TOKEN_LENGTH - 1)
            {
                tokens[token_count].value[j++] = input[i++];
            }

            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = TOKEN_NUMBER;

            token_count++;
            continue;
        }

        /* Operator */
        if (is_operator(input[i]))
        {
            tokens[token_count].type = TOKEN_OPERATOR;

            tokens[token_count].value[0] = input[i];
            tokens[token_count].value[1] = '\0';

            token_count++;
            i++;
            continue;
        }

        /* Word / command / identifier */
        if (isalpha((unsigned char)input[i]) ||
            input[i] == '_' ||
            input[i] == '.' ||
            input[i] == '/')
        {
            int j = 0;

            while (input[i] != '\0' &&
                   !isspace((unsigned char)input[i]) &&
                   input[i] != '|' &&
                   input[i] != '<' &&
                   input[i] != '>' &&
                   input[i] != ';' &&
                   input[i] != '"' &&
                   input[i] != '\'' &&
                   !is_operator(input[i]) &&
                   j < MAX_TOKEN_LENGTH - 1)
            {
                tokens[token_count].value[j++] = input[i++];
            }

            tokens[token_count].value[j] = '\0';
            tokens[token_count].type = TOKEN_WORD;

            token_count++;
            continue;
        }

        /* Unknown character */
        tokens[token_count].type = TOKEN_UNKNOWN;

        tokens[token_count].value[0] = input[i];
        tokens[token_count].value[1] = '\0';

        token_count++;
        i++;
    }

    /* EOF token */
    tokens[token_count].type = TOKEN_EOF;
    strcpy(tokens[token_count].value, "EOF");

    token_count++;

    return token_count;
}
