/* -*- indented-text -*- */
%{
#include "y.tab.h"
#include <string.h>

#ifdef DOESNT_HAVE_STRDUP
char *strdup(const char *s);
#endif

#define ECHO
%}
NL              [\n]
PIPE            [|]
OUTPUT_REDIR    [>]
INPUT_REDIR     [<]
BG              [&]
CHAR            [0-9a-zA-Z_/\-\*\.=]
QUOTED_CHAR     [^"]
SPACE           [\032]
%% 

exit               { ECHO; return EXIT; }
wait               { ECHO; return WAIT; }
kill               { ECHO; return KILL; }
{NL}               { ECHO; return 0; }
{BG}               { ECHO; return BACKGROUND; }
{PIPE}             { ECHO; return PIPE;}
{OUTPUT_REDIR}     { ECHO; return OUTPUT_REDIR; }
{INPUT_REDIR}      { ECHO; return INPUT_REDIR; }
{CHAR}+            { ECHO; yylval.string = yytext; return STRING; }
\'{QUOTED_CHAR}+\' {
  ECHO;
  /* strip quotes quotes */
  yylval.string = yytext + 1;
  yylval.string[yyleng - 2] = '\0';
  return STRING;
}
\"{QUOTED_CHAR}+\" {
  ECHO;
  /* strip quotes quotes */
  yylval.string = yytext + 1;
  yylval.string[yyleng - 2] = '\0';
  return STRING;
}
.;
%%
