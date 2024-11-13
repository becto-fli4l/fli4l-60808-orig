/*----------------------------------------------------------------------------
 *  squeeze.l - functions for squeezing shell scripts
 *
 *  Copyright (c) 2005-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

%option noinput
%option nounput
%option noyy_top_state

%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
    void squeeze_lex_init(FILE * in, FILE * out);
    void squeeze_print(char *);
    void squeeze_my_puts (char *);
    void squeeze_new_state (char c, char out);
    void squeeze_hash_bang (char *);
    void squeeze_here_doc_start (char * text);
    int squeeze_here_doc_print (char * text);
    static int nl;
    static int first;
%}

%x sq_str dq_str bq_str quote here_doc
%option stack

COMMENT (^[[:space:]]*#[^\r\n]*)|([[:space:]]+#[^\r\n]*)
%%
<*>\\           {
                    squeeze_new_state('\\', *yytext);
                }
<quote>[^\r\n]  {
                    squeeze_new_state('\\', *yytext);
                }
<quote>[\r]     {   /* ignore dos line end */
                }
<quote>[\n]     {
                    squeeze_new_state('\\', '\n');
                    nl = 1;
                }

<INITIAL>['"`]  {
                    squeeze_new_state(*yytext, *yytext);
                }

<sq_str>[^'\\\r\n]* {
                    squeeze_my_puts (yytext);
                }

<sq_str>[']     {
                    squeeze_new_state(*yytext, *yytext);
                }

<dq_str>[^"`\\\r\n]*    {
                    squeeze_my_puts (yytext);
                }


<dq_str>["`]    {
                    squeeze_new_state(*yytext, *yytext);
                }

<bq_str>[^'`"\\\r\n]*   {
                    squeeze_print (yytext);
                }

<bq_str>[`"']   {
                    squeeze_new_state(*yytext, *yytext);
                }


<sq_str,dq_str,bq_str>[\r]* {}
<sq_str,dq_str,bq_str>[\n]* {squeeze_my_puts (yytext);}

^#![^\r\n]*     { squeeze_hash_bang (yytext); }
.*<<[^\r\n#]*   {
                        squeeze_here_doc_start (yytext);
                        yy_push_state(here_doc);
                }
<here_doc>[^\n\r]*      {
                        if (squeeze_here_doc_print (yytext))
                                yy_pop_state();
                }
<here_doc>[\n]  {       ECHO; }
<here_doc>[\r]* {}

^[[:space:]]*#[^\r\n]*  {}
[[:space:]]+#[^\r\n]*   {}

[^\r\n\\#'"`]*[^[:space:]\r\n\\#'"`]+   {
                    squeeze_print (yytext);
                }
[[:blank:]]+$   { /* kill trailing spaces */ }
^[[:blank:]]+   { if (nl) fputc (' ', yyout); /* kill leading  spaces */ }
[[:blank:]]+    { squeeze_print(" "); }
[\r\n]*         {
                        if (nl) { fputc ('\n', yyout); nl = 0; }
                }


%%

void squeeze_lex_init(FILE * in, FILE * out)
{
    nl = 0;
    first = 1;
    yyin = in;
    yyout = out;
    BEGIN(INITIAL);
}

int yywrap ()
{
    return 1;
}
static char here_token[128];

void squeeze_hash_bang (char * text)
{
    if (first)
        squeeze_print (text);
}
void squeeze_here_doc_start (char * text)
{
    char * p = strstr (text, "<<");
    char * q = here_token;
    if (!p)
    {
        fprintf (stderr, "failed to locate << in here doc statement\n");
        abort ();
    }
    p += 2;
    if (*p == '-')
        p++;
    while (*p && isspace ((int) *p))
        p++;
    while (*p && (!isspace ((int) *p) && *p != '>'))
        *q++ = *p++;
    *q = 0;
    squeeze_print (text);
}
int squeeze_here_doc_print (char * text)
{
    squeeze_my_puts(text);
    if (text == strstr(text, here_token)) {
      return 1;
    }
    else
      return 0;

}
void squeeze_print (char * p)
{
  while (*p) {
    if (isblank((int) *p)) {
      if (nl)
      fputc (' ', yyout);
      p++;
      while (isblank ((int) *p))
        p++;
    } else {
      fputc (*p++, yyout);
      nl=1;
    }
  }
}
void squeeze_my_puts (char * text)
{
    first = 0;
    nl=1;
    fputs (text, yyout);
}
void squeeze_new_state (char c, char out)
{
    int state;

    if (out)
    {
        fputc(out, yyout);
        if (c == '\n')
            nl=0;
        else
            nl=1;
    }

    switch (c)
    {
    case '\'': state = sq_str; break;
    case '"': state = dq_str; break;
    case '`': state = bq_str; break;
    case '\\': state = quote; break;
    default:
        fprintf(stderr, "unknown state char '%c' (%d)\n", c, (int)c);
        abort();
        break;
    }
    if (YY_START == state)
    {
        yy_pop_state ();
#ifdef FLEX_DEBUG
        if (yy_flex_debug)
            fprintf(stderr, "leaving state %d (%d -> %d)\n",
                    state, state, YY_START);
#endif
    }
    else
    {
#ifdef FLEX_DEBUG
        if (yy_flex_debug)
            fprintf(stderr, "entering state %d (%d -> %d)\n",
                    state, YY_START, state);
#endif
        yy_push_state (state);
    }
}
