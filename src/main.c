#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "lexer.h"
#include "token.h"

#define HISTORY_SIZE 50

char history[HISTORY_SIZE][MAX_INPUT_LENGTH];
int history_count = 0;


/* =========================
   ADD COMMAND TO HISTORY
   ========================= */
void add_history(const char *command)
{
    if (strlen(command) == 0)
        return;

    /* Don't store consecutive duplicates */
    if (history_count > 0 &&
        strcmp(history[history_count - 1], command) == 0)
        return;

    if (history_count < HISTORY_SIZE)
    {
        strcpy(history[history_count], command);
        history_count++;
    }
    else
    {
        /* Remove oldest command */
        for (int i = 1; i < HISTORY_SIZE; i++)
        {
            strcpy(history[i - 1], history[i]);
        }

        strcpy(history[HISTORY_SIZE - 1], command);
    }
}


/* =========================
   SHOW HISTORY COMMAND
   ========================= */
void show_history()
{
    if (history_count == 0)
    {
        printf("No commands in history.\n");
        return;
    }

    printf("\nCommand History\n");
    printf("-----------------------------\n");

    for (int i = 0; i < history_count; i++)
    {
        printf("%d  %s\n", i + 1, history[i]);
    }

    printf("-----------------------------\n\n");
}


/* =========================
   CLEAR CURRENT INPUT LINE
   ========================= */
void clear_line(char *input, int *pos)
{
    while (*pos > 0)
    {
        printf("\b \b");
        (*pos)--;
    }

    input[0] = '\0';
}


/* =========================
   READ COMMAND
   SUPPORTS ↑ AND ↓
   ========================= */
void read_command(char *input)
{
    struct termios oldt, newt;

    /* Save terminal settings */
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;

    /* Disable canonical mode and echo */
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int pos = 0;

    /*
       history_index:
       history_count = position after newest command
    */
    int history_index = history_count;

    input[0] = '\0';

    while (1)
    {
        char c;

        if (read(STDIN_FILENO, &c, 1) <= 0)
            break;


        /* =========================
           ENTER
           ========================= */
        if (c == '\n' || c == '\r')
        {
            input[pos] = '\0';

            printf("\n");

            break;
        }


        /* =========================
           BACKSPACE
           ========================= */
        if (c == 127 || c == 8)
        {
            if (pos > 0)
            {
                pos--;

                input[pos] = '\0';

                printf("\b \b");

                fflush(stdout);
            }

            continue;
        }


        /* =========================
           ARROW KEYS
           ========================= */
        if (c == 27)
        {
            char bracket;
            char direction;

            if (read(STDIN_FILENO, &bracket, 1) <= 0)
                continue;

            if (read(STDIN_FILENO, &direction, 1) <= 0)
                continue;


            /* =========================
               UP ARROW
               ========================= */
            if (bracket == '[' && direction == 'A')
            {
                if (history_count > 0 &&
                    history_index > 0)
                {
                    history_index--;

                    clear_line(input, &pos);

                    strcpy(input, history[history_index]);

                    pos = strlen(input);

                    printf("%s", input);

                    fflush(stdout);
                }

                continue;
            }


            /* =========================
               DOWN ARROW
               ========================= */
            if (bracket == '[' && direction == 'B')
            {
                if (history_index < history_count - 1)
                {
                    history_index++;

                    clear_line(input, &pos);

                    strcpy(input, history[history_index]);

                    pos = strlen(input);

                    printf("%s", input);

                    fflush(stdout);
                }
                else if (history_index == history_count - 1)
                {
                    history_index = history_count;

                    clear_line(input, &pos);
                }

                continue;
            }

            continue;
        }


        /* =========================
           NORMAL CHARACTER
           ========================= */
        if (pos < MAX_INPUT_LENGTH - 1)
        {
            input[pos] = c;

            pos++;

            input[pos] = '\0';

            putchar(c);

            fflush(stdout);
        }
    }


    /* Restore terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


/* =========================
   MAIN
   ========================= */
int main()
{
    char input[MAX_INPUT_LENGTH];

    Token tokens[MAX_TOKENS];

    printf("====================================\n");
    printf("        SHELLFORGE - MILESTONE 2\n");
    printf("          TOKENIZER / LEXER\n");
    printf("====================================\n");

    while (1)
    {
        printf("shellforge> ");

        fflush(stdout);


        /* Read command */
        read_command(input);


        /* Empty command */
        if (strlen(input) == 0)
            continue;


        /* =========================
           EXIT
           ========================= */
        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting ShellForge...\n");

            break;
        }


        /* =========================
           HISTORY
           ========================= */
        if (strcmp(input, "history") == 0)
        {
            show_history();

            continue;
        }


        /* Save command */
        add_history(input);


        /* =========================
           TOKENIZATION
           ========================= */
        int count = tokenize(input, tokens);


        printf("\nTokens:\n");

        printf("------------------------------------\n");


        for (int i = 0; i < count; i++)
        {
            printf("Token %-3d | %-15s | %s\n",
                   i + 1,
                   token_type_name(tokens[i].type),
                   tokens[i].value);

            if (tokens[i].type == TOKEN_EOF)
                break;
        }


        printf("------------------------------------\n\n");
    }

    return 0;
}
