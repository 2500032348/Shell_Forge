#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "executor.h"
#include "builtin.h"

int execute_command(Command *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0)
        return -1;

    if (is_builtin(cmd))
        return execute_builtin(cmd);

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        if (cmd->input != NULL)
        {
            int fd = open(cmd->input, O_RDONLY);

            if (fd < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }

            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (cmd->output != NULL)
        {
            int flags = O_WRONLY | O_CREAT;

            if (cmd->append)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;

            int fd = open(cmd->output, flags, 0644);

            if (fd < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(cmd->argv[0], cmd->argv);

        perror("Shellforge");
        exit(EXIT_FAILURE);
    }

    if (cmd->background)
    {
        printf("[background pid %d]\n", pid);
        return 0;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
    {
        fprintf(stderr,
                "Process terminated by signal %d\n",
                WTERMSIG(status));
        return -1;
    }

    return 0;
}


int execute_pipeline(Pipeline *pipeline)
{
    int n;
    int pipes[MAX_COMMANDS - 1][2];
    pid_t pids[MAX_COMMANDS];
    int last_status = 0;

    if (pipeline == NULL)
        return -1;

    n = pipeline->command_count;

    if (n <= 0)
        return -1;

    if (n == 1)
        return execute_command(&pipeline->commands[0]);

    for (int i = 0; i < n - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return -1;
        }
    }

    for (int i = 0; i < n; i++)
    {
        Command *cmd = &pipeline->commands[i];

        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");

            for (int j = 0; j < n - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return -1;
        }

        if (pids[i] == 0)
        {
            /* Input from previous pipe */
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /* Output to next pipe */
            if (i < n - 1)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /* Input redirection */
            if (cmd->input != NULL)
            {
                int fd = open(cmd->input, O_RDONLY);

                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                close(fd);
            }

            /* Output redirection */
            if (cmd->output != NULL)
            {
                int flags = O_WRONLY | O_CREAT;

                if (cmd->append)
                    flags |= O_APPEND;
                else
                    flags |= O_TRUNC;

                int fd = open(cmd->output, flags, 0644);

                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                close(fd);
            }

            /* Close all pipe descriptors */
            for (int j = 0; j < n - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(cmd->argv[0], cmd->argv);

            perror("Shellforge");
            exit(EXIT_FAILURE);
        }
    }

    /* Parent closes all pipes */
    for (int i = 0; i < n - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Wait for all children */
    for (int i = 0; i < n; i++)
    {
        int status;

        if (waitpid(pids[i], &status, 0) == -1)
        {
            perror("waitpid");
            return -1;
        }

        if (i == n - 1)
        {
            if (WIFEXITED(status))
                last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                last_status = -1;
        }
    }

    return last_status;
}
