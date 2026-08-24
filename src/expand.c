#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expand.h"

static void expand_variable(Token *token)
{
    if (token->type != TOKEN_WORD)
        return;

    char result[MAX_TOKEN_LEN];
    result[0] = '\0';

    char *p = token->value;

    while (*p != '\0') {

        if (*p == '$') {

            p++;

            char variable[128];
            int i = 0;

            while (*p != '\0' &&
                   ((*p >= 'A' && *p <= 'Z') ||
                    (*p >= 'a' && *p <= 'z') ||
                    (*p >= '0' && *p <= '9') ||
                    *p == '_')) {

                if (i < 127)
                    variable[i++] = *p;

                p++;
            }

            variable[i] = '\0';

            if (i > 0) {

                char *value = getenv(variable);

                if (value != NULL) {
                    strncat(result,
                            value,
                            MAX_TOKEN_LEN - strlen(result) - 1);
                }

            } else {
                strncat(result,
                        "$",
                        MAX_TOKEN_LEN - strlen(result) - 1);
            }

        } else {

            char temp[2];

            temp[0] = *p;
            temp[1] = '\0';

            strncat(result,
                    temp,
                    MAX_TOKEN_LEN - strlen(result) - 1);

            p++;
        }
    }

    strncpy(token->value,
            result,
            MAX_TOKEN_LEN - 1);

    token->value[MAX_TOKEN_LEN - 1] = '\0';
}

void expand_tokens(Token tokens[], int *count)
{
    for (int i = 0; i < *count; i++) {

        if (tokens[i].type == TOKEN_WORD) {
            expand_variable(&tokens[i]);
        }
    }
}
