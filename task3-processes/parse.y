/* -*- indented-text -*- */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
int yylex(void);

static void append_argv(struct command *p, char* new_val);
static struct command *new_command();
static void yyerror(const char *s);
int yywrap(void);
%}

%union {
       char *string;
}


%start cmd_line
%token <string> EXIT WAIT KILL PIPE INPUT_REDIR OUTPUT_REDIR STRING NL BACKGROUND


%%
cmd_line    :
  | EXIT { current_builtin = BUILTIN_EXIT; builtin_arg = NULL; }
  | EXIT STRING { current_builtin = BUILTIN_EXIT; builtin_arg = strdup($2); }
  | WAIT { current_builtin = BUILTIN_WAIT; }
  | WAIT STRING { current_builtin = BUILTIN_WAIT; builtin_arg = strdup($2); }
  | KILL STRING { current_builtin = BUILTIN_KILL; builtin_arg = strdup($2); }
  | pipeline background
  ;

background :
     BACKGROUND { current_pipeline.background = 1; }
  |             { current_pipeline.background = 0; }
  ;

simple : command redir
  ;

command :
    command STRING {
      if (!current_command) {
         current_command = new_command();
         previous_command->next = current_command;
      }
      append_argv(current_command, $2);
    }
  | STRING {
      if (!current_command) {
         current_command = new_command();
         previous_command->next = current_command;
      }
      append_argv(current_command, $1);
    }
  ;

redir  :
    input_redir output_redir
  ;

output_redir:
     OUTPUT_REDIR STRING {
       current_command->output_redir = strdup($2);
       if (!current_command->output_redir) {
         fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
         exit(1);
       }
     }
  |                      { current_command->output_redir = NULL; }
  ;

input_redir:
    INPUT_REDIR STRING {
      current_command->input_redir = strdup($2);
      if (!current_command->input_redir) {
        fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
        exit(1);
      }
    }
  |                    { current_command->input_redir = NULL; }
  ;
pipeline :
    pipeline PIPE simple {
      previous_command = current_command;
      current_command = NULL;
    }
  | simple               {
      previous_command = current_command;
      current_command = NULL;
    }
  ;
%%

static void append_argv(struct command *c, char* new_val) {
 char *arg = strdup(new_val);
 if (!arg) {
   fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
   exit(1);
 }
 if (!c->argv) {
   c->argv = calloc(8, sizeof(char*));
   if (!c->argv) {
     fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
     exit(1);
   }
   c->argv_cap = 8;
 }
 if ((c->argc + 1) > c->argv_cap) {
   c->argv_cap *= 2;
   c->argv = realloc(c->argv, sizeof(char**) * c->argv_cap);
   if (!c->argv) {
     fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
     exit(1);
   }
 }
 c->argv[c->argc++] = arg;
 c->argv[c->argc] = NULL;
}

static struct command *new_command() {
  struct command *c = calloc(1, sizeof(struct command));
  if (!c) {
    fprintf(stderr, "%s:%d: cannot allocate memory\n", __FILE__, __LINE__);
    exit(1);
  }
  return c;
}

static void yyerror(const char *s) {
  fprintf(stderr, "error %s\n", s);
}

int yywrap(void) {
  eof = 1;
  return 1;
}
