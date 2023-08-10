#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

struct pipeline current_pipeline;
struct command *current_command = NULL, *previous_command;
enum builtin_type current_builtin = BUILTIN_NONE;
char *builtin_arg = NULL;
int eof;

static int check_consistency(struct pipeline *pipeline) {
  int ret = 1;
  for (struct command *c = &pipeline->first_command; c; c = c->next) {
    if (c->output_redir && c->next) {
      fprintf(stderr, "Ambiguous output redirect for command %s.\n",
              c->argv[0]);
      ret = 0;
    }
  }
  return ret;
}

static void free_command_args(struct command *c) {
  if (c->argv) {
    for (size_t i = 0; i < c->argc; i++) {
      free(c->argv[i]);
    }
    free(c->argv);
    c->argv = NULL;
  }
  if (c->input_redir) {
    free(c->input_redir);
    c->input_redir = NULL;
  }
  if (c->output_redir) {
    free(c->output_redir);
    c->output_redir = NULL;
  }
}

static void free_pipeline_cmds(struct pipeline *p) {
  struct command *c = p->first_command.next;
  free_command_args(&p->first_command);
  while (c) {
    struct command *next = c->next;
    free_command_args(c);
    free(c);
    c = next;
  }
}


// uncomment this for more verbose output
//#define DEBUG

int main(void) {
  while (1) {
    current_builtin = BUILTIN_NONE;
    builtin_arg = NULL;
    current_pipeline.background = 0;
    memset(&current_pipeline, 0, sizeof(current_pipeline));
    current_command = &current_pipeline.first_command;
    previous_command = NULL;

    if (isatty(fileno(stdin))) {
      printf("shell> ");
    }
    yyparse();

    if (eof) {
      exit(0);
    }

    if (current_builtin != BUILTIN_NONE) {
#ifdef DEBUG
      char *name = NULL;
      switch (current_builtin) {
        case BUILTIN_EXIT: { name = "exit"; break; }
        case BUILTIN_WAIT: { name = "wait"; break; }
        case BUILTIN_KILL: { name = "kill"; break; }
        case BUILTIN_NONE: { name = "none"; break; }
      };
      printf("builtin %s(%s)\n\n", name, builtin_arg ? builtin_arg : "");
      fflush(stdout);
#endif
      run_builtin(current_builtin, builtin_arg);
      if (builtin_arg) {
        free(builtin_arg);
      }
    } else if (check_consistency(&current_pipeline)) {
#ifdef DEBUG
      int i = 0;
      for (struct command *c = &current_pipeline.first_command; c && c->argc;
           c = c->next) {
        i++;
        printf("command %d\n", i);
        for (size_t j = 0; j < (size_t)c->argc; j++) {
          printf("  argv[%d]: %s\n", i, c->argv[j]);
        }
        if (c->input_redir) {
          printf("  input_redir: %s\n", c->input_redir);
        }
        if (c->output_redir) {
          printf("  output_redir: %s\n", c->output_redir);
        }
      }
      if (current_pipeline.background) {
        printf("background execution\n");
      }
#endif

      if (current_pipeline.first_command.argc != 0) {
        run_pipeline(&current_pipeline);
        free_pipeline_cmds(&current_pipeline);
      }
    }
  }

  return 0;
}
