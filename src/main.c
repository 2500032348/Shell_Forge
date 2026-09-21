#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "token.h"

int main(void)
{
    char input[MAX_INPUT_LENGTH];

    while (1)
    {
        printf("shellforge$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            continue;

        Token tokens[MAX_TOKENS];

        int token_count = tokenize(input, tokens);

        Pipeline pipeline;

        if (!parse_pipeline(tokens, token_count, &pipeline))
        {
            fprintf(stderr, "Parsing error\n");
            continue;
        }

        execute_pipeline(&pipeline);

        free_pipeline(&pipeline);
    }

    return 0;
}
