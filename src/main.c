#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "expand.h"

#define INPUT_SIZE 1024

int main(void)
{
    char line[INPUT_SIZE];

    printf("================================\n");
    printf("        Shellforge\n");
    printf(" A Unix Style Shell written in C\n");
    printf("================================\n");

    while (1) {

        printf("shellforge$ ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';

        /* Empty command */
        if (strlen(line) == 0) {
            continue;
        }

        /* Exit */
        if (strcmp(line, "exit") == 0) {
            break;
        }

        Token tokens[MAX_TOKENS];

        int token_count = tokenize(line, tokens);

        /*
         * Expand variables before parsing.
         */
        expand_tokens(tokens, &token_count);

        /*
         * Display tokens
         */
        print_tokens(tokens, token_count);

        /*
         * Build pipeline
         */
        Pipeline pipeline;

        parse_pipeline(tokens,
                       token_count,
                       &pipeline);

        /*
         * Display pipeline
         */
        print_pipeline(&pipeline);

        /*
         * Free dynamically allocated memory
         */
        free_pipeline(&pipeline);
    }

    return 0;
}
