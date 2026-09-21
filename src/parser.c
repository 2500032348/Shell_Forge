#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

int parse_pipeline(Token tokens[], int token_count, Pipeline *pipeline)
{
    memset(pipeline, 0, sizeof(Pipeline));

    int cmd = 0;
    int argc = 0;

    pipeline->command_count = 1;

    for (int i = 0; i < MAX_COMMANDS; i++)
    {
        pipeline->commands[i].argc = 0;
        pipeline->commands[i].argv[0] = NULL;
        pipeline->commands[i].input = NULL;
        pipeline->commands[i].output = NULL;
        pipeline->commands[i].append = 0;
        pipeline->commands[i].background = 0;
    }

    for (int i = 0; i < token_count; i++)
    {
        Token *token = &tokens[i];

        if (token->type == TOKEN_END)
        {
            break;
        }

        /* Normal command/argument */
        if (token->type == TOKEN_WORD)
        {
            if (argc < MAX_ARGS - 1)
            {
                pipeline->commands[cmd].argv[argc] =
                    strdup(token->value);

                argc++;

                pipeline->commands[cmd].argc = argc;
                pipeline->commands[cmd].argv[argc] = NULL;
            }

            continue;
        }

        /* Pipe */
        if (token->type == TOKEN_PIPE)
        {
            if (argc == 0)
            {
                fprintf(stderr, "Shellforge: invalid pipe\n");
                return 0;
            }

            if (cmd < MAX_COMMANDS - 1)
            {
                cmd++;
                pipeline->command_count++;
                argc = 0;
            }

            continue;
        }

        /* Input redirection */
        if (token->type == TOKEN_INPUT)
        {
            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD)
            {
                pipeline->commands[cmd].input =
                    strdup(tokens[++i].value);
            }
            else
            {
                fprintf(stderr,
                        "Shellforge: expected filename after <\n");
                return 0;
            }

            continue;
        }

        /* Output redirection */
        if (token->type == TOKEN_OUTPUT)
        {
            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD)
            {
                pipeline->commands[cmd].output =
                    strdup(tokens[++i].value);

                pipeline->commands[cmd].append = 0;
            }
            else
            {
                fprintf(stderr,
                        "Shellforge: expected filename after >\n");
                return 0;
            }

            continue;
        }

        /* Append redirection */
        if (token->type == TOKEN_APPEND)
        {
            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD)
            {
                pipeline->commands[cmd].output =
                    strdup(tokens[++i].value);

                pipeline->commands[cmd].append = 1;
            }
            else
            {
                fprintf(stderr,
                        "Shellforge: expected filename after >>\n");
                return 0;
            }

            continue;
        }

        /* Background */
        if (token->type == TOKEN_BACKGROUND)
        {
            pipeline->commands[cmd].background = 1;
            continue;
        }
    }

    if (pipeline->commands[cmd].argc == 0)
    {
        pipeline->command_count--;
    }

    return 1;
}

void print_pipeline(Pipeline *pipeline)
{
    printf("\n========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++)
    {
        Command *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("------------------------------\n");

        printf("Arguments\n");

        for (int j = 0; j < cmd->argc; j++)
        {
            printf("argv[%d] = %s\n", j, cmd->argv[j]);
        }

        printf("Input     : %s\n",
               cmd->input ? cmd->input : "None");

        printf("Output    : %s\n",
               cmd->output ? cmd->output : "None");

        printf("Append    : %s\n",
               cmd->append ? "Yes" : "No");

        printf("Background: %s\n",
               cmd->background ? "Yes" : "No");
    }

    printf("==============================\n");
}

void free_pipeline(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++)
    {
        Command *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }

        cmd->argc = 0;

        if (cmd->input != NULL)
        {
            free(cmd->input);
            cmd->input = NULL;
        }

        if (cmd->output != NULL)
        {
            free(cmd->output);
            cmd->output = NULL;
        }
    }

    pipeline->command_count = 0;
}
