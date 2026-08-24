#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "parser.h"

static void add_token(Token tokens[], int *count,
                      TokenType type, const char *value)
{
    if (*count >= MAX_TOKENS - 1)
        return;

    tokens[*count].type = type;

    if (value != NULL)
        strncpy(tokens[*count].value, value, MAX_TOKEN_LEN - 1);
    else
        tokens[*count].value[0] = '\0';

    tokens[*count].value[MAX_TOKEN_LEN - 1] = '\0';

    (*count)++;
}

int tokenize(const char *line, Token tokens[])
{
    int count = 0;
    int i = 0;

    while (line[i] != '\0' && line[i] != '\n') {

        /* Skip spaces */
        if (isspace((unsigned char)line[i])) {
            i++;
            continue;
        }

        /* Pipe */
        if (line[i] == '|') {
            add_token(tokens, &count, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (line[i] == '<') {
            add_token(tokens, &count, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Output / append redirection */
        if (line[i] == '>') {

            if (line[i + 1] == '>') {
                add_token(tokens, &count, TOKEN_APPEND, ">>");
                i += 2;
            } else {
                add_token(tokens, &count, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Background */
        if (line[i] == '&') {
            add_token(tokens, &count, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        /* Word */
        char word[MAX_TOKEN_LEN];
        int j = 0;

        while (line[i] != '\0' &&
               line[i] != '\n' &&
               !isspace((unsigned char)line[i]) &&
               line[i] != '|' &&
               line[i] != '<' &&
               line[i] != '>' &&
               line[i] != '&') {

            if (j < MAX_TOKEN_LEN - 1) {
                word[j++] = line[i];
            }

            i++;
        }

        word[j] = '\0';

        if (j > 0) {
            add_token(tokens, &count, TOKEN_WORD, word);
        }
    }

    add_token(tokens, &count, TOKEN_END, "END");

    return count;
}

static const char *token_name(TokenType type)
{
    switch (type) {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_INPUT:
            return "INPUT";

        case TOKEN_OUTPUT:
            return "OUTPUT";

        case TOKEN_APPEND:
            return "APPEND";

        case TOKEN_BACKGROUND:
            return "BACKGROUND";

        case TOKEN_END:
            return "END";

        default:
            return "UNKNOWN";
    }
}

void print_tokens(Token tokens[], int count)
{
    printf("\n------------ TOKENS ------------\n");

    for (int i = 0; i < count; i++) {
        printf("%2d : %-12s %s\n",
               i,
               token_name(tokens[i].type),
               tokens[i].value);
    }

    printf("--------------------------------\n");
}

int parse_pipeline(Token tokens[], int token_count, Pipeline *pipeline)
{
    memset(pipeline, 0, sizeof(Pipeline));

    pipeline->command_count = 1;

    int cmd = 0;
    int argc = 0;

    for (int i = 0; i < MAX_COMMANDS; i++) {
        pipeline->commands[i].argv[0] = NULL;
        pipeline->commands[i].input = NULL;
        pipeline->commands[i].output = NULL;
        pipeline->commands[i].append = 0;
        pipeline->commands[i].background = 0;
    }

    for (int i = 0; i < token_count; i++) {

        Token *token = &tokens[i];

        if (token->type == TOKEN_END) {
            break;
        }

        /* Normal argument */
        if (token->type == TOKEN_WORD) {

            if (argc < MAX_ARGS - 1) {

                pipeline->commands[cmd].argv[argc] =
                    strdup(token->value);

                argc++;

                pipeline->commands[cmd].argv[argc] = NULL;
            }

            continue;
        }

        /* Pipe */
        if (token->type == TOKEN_PIPE) {

            if (cmd < MAX_COMMANDS - 1) {
                cmd++;
                pipeline->command_count++;
                argc = 0;
            }

            continue;
        }

        /* Input redirection */
        if (token->type == TOKEN_INPUT) {

            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD) {

                pipeline->commands[cmd].input =
                    strdup(tokens[++i].value);
            }

            continue;
        }

        /* Output redirection */
        if (token->type == TOKEN_OUTPUT) {

            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD) {

                pipeline->commands[cmd].output =
                    strdup(tokens[++i].value);

                pipeline->commands[cmd].append = 0;
            }

            continue;
        }

        /* Append redirection */
        if (token->type == TOKEN_APPEND) {

            if (i + 1 < token_count &&
                tokens[i + 1].type == TOKEN_WORD) {

                pipeline->commands[cmd].output =
                    strdup(tokens[++i].value);

                pipeline->commands[cmd].append = 1;
            }

            continue;
        }

        /* Background */
        if (token->type == TOKEN_BACKGROUND) {
            pipeline->commands[cmd].background = 1;
            continue;
        }
    }

    return 0;
}

void print_pipeline(Pipeline *pipeline)
{
    printf("\n========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++) {

        Command *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("------------------------------\n");

        printf("Arguments\n");

        for (int j = 0; cmd->argv[j] != NULL; j++) {
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
    for (int i = 0; i < pipeline->command_count; i++) {

        Command *cmd = &pipeline->commands[i];

        for (int j = 0; cmd->argv[j] != NULL; j++) {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }

        if (cmd->input != NULL) {
            free(cmd->input);
            cmd->input = NULL;
        }

        if (cmd->output != NULL) {
            free(cmd->output);
            cmd->output = NULL;
        }
    }

    pipeline->command_count = 0;
}
