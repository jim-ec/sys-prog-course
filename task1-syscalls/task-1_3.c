#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s executable arguments...\n", argv[0]);
        return -1;
    }
    
    int pid = fork();
    if (pid == -1) {
        perror("Cannot create child process");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Child
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "Could not execute `%s`: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    else {
        // Parent
        waitpid(pid, 0, 0);

        for (;;) {
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) { return 0; }
            waitpid(pid, 0, 0);

            struct user_regs_struct registers;
            ptrace(PTRACE_GETREGS, pid, 0, &registers);
            bool printResult = false;

            if (registers.orig_rax == 0) {
                fprintf(stderr, "read(%d, %p, %llu)", (int) registers.rdi, (void *) registers.rsi, registers.rdx);
                printResult = true;
            }
            else if (registers.orig_rax == 1) {
                fprintf(stderr, "write(%d, %p, %llu)", (int) registers.rdi, (void *) registers.rsi, registers.rdx);
                printResult = true;
            }

            ptrace(PTRACE_SYSCALL, pid, 0, 0);
            waitpid(pid, 0, 0);
            ptrace(PTRACE_GETREGS, pid, 0, &registers);
            if (printResult) {
                fprintf(stderr, " = %llu\n", registers.rax);
            }
        }
    }
}
