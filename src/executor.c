#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "executor.h"
#include "builtin.h"


/*
 * SIGCHLD handler
 * Prevents zombie background processes.
 */
static void sigchld_handler(int sig)
{
    int saved_errno = errno;

    (void)sig;

    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        /* Reap finished background children */
    }

    errno = saved_errno;
}


/*
 * Setup SIGCHLD handler.
 */
void setup_background_handler(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = sigchld_handler;

    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
    }
}


/*
 * Execute a single command.
 */
int execute_command(Command *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL || cmd->argc == 0)
        return -1;


    /*
     * Background built-in:
     * Run the built-in inside a child.
     */
    if (is_builtin(cmd) && cmd->background)
    {
        pid = fork();

        if (pid < 0)
        {
            perror("fork");
            return -1;
        }

        if (pid == 0)
        {
            int null_fd;

            null_fd = open("/dev/null", O_RDONLY);

            if (null_fd >= 0)
            {
                dup2(null_fd, STDIN_FILENO);
                close(null_fd);
            }

            execute_builtin(cmd);

            _exit(0);
        }

        printf("[Background PID: %d]\n", pid);
        return 0;
    }


    /*
     * Normal built-in command.
     */
    if (is_builtin(cmd))
    {
        return execute_builtin(cmd);
    }


    /*
     * Create child.
     */
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }


    /*
     * Child process.
     */
    if (pid == 0)
    {
        /*
         * Background command:
         * Redirect stdin to /dev/null.
         */
        if (cmd->background)
        {
            int null_fd = open("/dev/null", O_RDONLY);

            if (null_fd < 0)
            {
                perror("open /dev/null");
                _exit(127);
            }

            if (dup2(null_fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                close(null_fd);
                _exit(127);
            }

            close(null_fd);
        }


        /*
         * Input redirection.
         */
        if (cmd->input != NULL)
        {
            int fd = open(cmd->input, O_RDONLY);

            if (fd < 0)
            {
                perror("open input");
                _exit(127);
            }

            if (dup2(fd, STDIN_FILENO) == -1)
            {
                perror("dup2 input");
                close(fd);
                _exit(127);
            }

            close(fd);
        }


        /*
         * Output redirection.
         */
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
                perror("open output");
                _exit(127);
            }

            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output");
                close(fd);
                _exit(127);
            }

            close(fd);
        }


        /*
         * Execute command.
         */
        execvp(cmd->argv[0], cmd->argv);

        perror("Shellforge");
        _exit(127);
    }


    /*
     * Background:
     * Parent does NOT wait.
     */
    if (cmd->background)
    {
        printf("[Background PID: %d]\n", pid);
        return 0;
    }


    /*
     * Foreground:
     * Parent waits.
     */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status))
    {
        fprintf(stderr,
                "Process terminated by signal %d\n",
                WTERMSIG(status));

        return -1;
    }

    return 0;
}


/*
 * Execute a complete pipeline.
 *
 * Example:
 *
 * ls -l | grep .c | wc -l
 *
 * Background:
 *
 * ls -l | grep .c &
 */
int execute_pipeline(Pipeline *pipeline)
{
    int n;
    int background;
    int pipes[MAX_COMMANDS - 1][2];
    pid_t pids[MAX_COMMANDS];

    if (pipeline == NULL)
        return -1;

    n = pipeline->command_count;

    if (n <= 0)
        return -1;


    /*
     * Check background flag of the LAST command.
     */
    background =
        pipeline->commands[n - 1].background;


    /*
     * Single command.
     */
    if (n == 1)
    {
        return execute_command(&pipeline->commands[0]);
    }


    /*
     * Create pipes.
     */
    for (int i = 0; i < n - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return -1;
        }
    }


    /*
     * Fork each command.
     */
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


        /*
         * Child.
         */
        if (pids[i] == 0)
        {
            /*
             * Background pipeline:
             * First command gets stdin from /dev/null.
             */
            if (background && i == 0)
            {
                int null_fd = open("/dev/null", O_RDONLY);

                if (null_fd < 0)
                {
                    perror("open /dev/null");
                    _exit(127);
                }

                if (dup2(null_fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    close(null_fd);
                    _exit(127);
                }

                close(null_fd);
            }


            /*
             * Input from previous pipe.
             */
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2 stdin");
                    _exit(127);
                }
            }


            /*
             * Output to next pipe.
             */
            if (i < n - 1)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2 stdout");
                    _exit(127);
                }
            }


            /*
             * Input redirection.
             * Explicit redirection overrides /dev/null.
             */
            if (cmd->input != NULL)
            {
                int fd = open(cmd->input, O_RDONLY);

                if (fd < 0)
                {
                    perror("open input");
                    _exit(127);
                }

                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("dup2 input");
                    close(fd);
                    _exit(127);
                }

                close(fd);
            }


            /*
             * Output redirection.
             */
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
                    perror("open output");
                    _exit(127);
                }

                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("dup2 output");
                    close(fd);
                    _exit(127);
                }

                close(fd);
            }


            /*
             * Close all pipe descriptors.
             */
            for (int j = 0; j < n - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }


            /*
             * Execute command.
             */
            execvp(cmd->argv[0], cmd->argv);

            perror("Shellforge");
            _exit(127);
        }
    }


    /*
     * Parent closes all pipe descriptors.
     */
    for (int i = 0; i < n - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }


    /*
     * Background pipeline:
     * Do NOT wait.
     *
     * Print PID of first process.
     */
    if (background)
    {
        printf("[Background Pipeline PID: %d]\n",
               pids[0]);

        return 0;
    }


    /*
     * Foreground pipeline:
     * Wait for every child.
     */
    int last_status = 0;

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
