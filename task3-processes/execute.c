#include "parse.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>


/// Runs the given command.
/// If `in_fd` is not `-1`, the spawned command will read from that file descriptor instead of this process' stdin.
int run_command(struct command *command, int in_fd)
{
    bool nonterminal = command->next != NULL;

    // The pipe the current command will write into, the and the next command will read from.
    // pipefd[0]: Reading end
    // pipefd[1]: Writing end
    int pipefd[2];

    // Only actually create a pipe if there is another command to communicate to.
    if (nonterminal)
    {
        if (-1 == pipe(pipefd))
        {
            fprintf(stderr, "[FATAL] Cannot create pipe: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    // Spawn a new process which will run the given command.
    int pid = fork();
    if (pid != 0)
    {
        // Parent process, recurse until the terminal command is reached.
        if (nonterminal)
        {
            close(pipefd[1]);
            int next_pid = run_command(command->next, pipefd[0]);
            waitpid(next_pid, NULL, 0);
            close(pipefd[0]);
        }
        return pid;
    }
    else
    {
        // Child process, running the command. Possibly redirect stdin and stdout to pipes or physical files.
        if (in_fd != -1)
        {
            // Let the command read from the pipe.
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (command->input_redir != NULL)
        {
            // Let the command read from a file.
            int fd = open(command->input_redir, O_RDONLY);
            if (fd < 0)
            {
                fprintf(stderr, "Cannot open file `%s` for reading: %s\n", command->input_redir, strerror(errno));
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (nonterminal)
        {
            // Let the command write into the pipe.
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        }
        if (command->output_redir != NULL)
        {
            // Let the command write into a file.
            int fd = open(command->output_redir, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
            if (fd < 0)
            {
                fprintf(stderr, "Cannot open file `%s` for writing: %s\n", command->output_redir, strerror(errno));
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Run the given command, using this process.
        execvp(command->argv[0], command->argv);
        fprintf(stderr, "Cannot run `%s`: %s\n", command->argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void run_pipeline(struct pipeline *pipeline)
{
    int pid = run_command(&pipeline->first_command, -1);
    if (pipeline->background)
    {
        fprintf(stderr, "Running as %d ...\n", pid);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}


void run_builtin(enum builtin_type builtin, char *builtin_arg)
{
    if (builtin == BUILTIN_WAIT)
    {
        if (builtin_arg != NULL)
        {
            int pid = atoi(builtin_arg);
            waitpid(pid, NULL, 0);
        }
        else
        {
            waitpid(0, NULL, 0);
        }
    }
    else if (builtin == BUILTIN_EXIT)
    {
        if (builtin_arg != NULL)
        {
            int code = atoi(builtin_arg);
            exit(code);
        }
        else
        {
            exit(0);
        }
    }
    else if (builtin == BUILTIN_KILL)
    {
        if (builtin_arg != NULL)
        {
            int pid = atoi(builtin_arg);
            kill(pid, SIGTERM);
        }
        else
        {
            fprintf(stderr, "Usage:  exit [pid]\n");
        }
    }
}
