#include <stddef.h>
#include <unistd.h>

// A command with in a pipeline
struct command {
  // The next command in the pipeline, NULL if there is no next command
  struct command *next;

  // Array arguments that should be used to call the command. argv[0] is the
  // program name that should be used to perform the path search.
  char **argv;
  size_t argv_cap;
  // number of arguments in argv
  int argc;
  // If not NULL, a file path that stdout should be connected.
  char *output_redir;
  // If not NULL, a file path that stdin should be connected.
  char *input_redir;
};

// Result of one line parsed by the shell
struct pipeline {
  // the first command to be executed
  struct command first_command;
  // Has the value `1` if user specified `&` at the end of a command line input.
  // If 0, than the shell should block until the pipeline is finished.
  // Otherwise it should proceed with the next command without waiting.
  char background;
};

enum builtin_type {
  // Used for the parser internally, never passed to `run_builtin`
  BUILTIN_NONE = 0,
  // `exit` builtin
  BUILTIN_EXIT = 1,
  // `wait` builtin
  BUILTIN_WAIT = 2,
  // `kill` builtin
  BUILTIN_KILL = 3
};

// Internal parser state
extern enum builtin_type current_builtin;
extern struct pipeline current_pipeline;
extern struct command *current_command, *previous_command;
extern int eof;
extern char* builtin_arg;

// Internal parser function
int yyparse(void);

// To be implemented, see README
void run_pipeline(struct pipeline *p);
// To be implemented, see README
void run_builtin(enum builtin_type builtin, char* builtin_arg);
