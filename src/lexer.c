#include <ctype.h>
#include <string.h>

#include "lexer.h"

int tokenize(const char *input, Token tokens[])
{
    int i = 0;
    int token_count = 0;

    while (input[i] != '\0' &&
           token_count < MAX_TOKENS - 1)
    {
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
            tokens[token_count].type = TOKEN_INPUT;
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
                tokens[token_count].type = TOKEN_APPEND;
                strcpy(tokens[token_count].value, ">>");
                i += 2;
            }
            else
            {
                tokens[token_count].type = TOKEN_OUTPUT;
                strcpy(tokens[token_count].value, ">");
                i++;
            }

            token_count++;
            continue;
        }

        /* Background */
        if (input[i] == '&')
        {
            tokens[token_count].type = TOKEN_BACKGROUND;
            strcpy(tokens[token_count].value, "&");
            token_count++;
            i++;
            continue;
        }

        /* Word */
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>' &&
               input[i] != '&' &&
               j < MAX_TOKEN_LEN - 1)
        {
            tokens[token_count].value[j++] = input[i++];
        }

        tokens[token_count].value[j] = '\0';
        tokens[token_count].type = TOKEN_WORD;

        token_count++;
    }

    tokens[token_count].type = TOKEN_END;
    strcpy(tokens[token_count].value, "END");
    token_count++;

    return token_count;
}
