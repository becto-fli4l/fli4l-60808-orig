/*----------------------------------------------------------------------------
 *  grammar.y - functions for parsing extended check scripts
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This file is part of fli4l.

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

%{
#include "libmkfli4l/tree.h"
#include "libmkfli4l/tree_struct.h"
#include "libmkfli4l/str.h"
#include "libmkfli4l/log.h"
#include "libmkfli4l/parse.h"

int yyerror(char const *s);
int yylex(void);
extern int yyline;
extern char *yytext;

%}

%error-verbose

/* BISON Declarations */
%union {
    int line;
    elem_t *elem;
}

/*

type    op1     op2     op3
if      cond    then    else
foreach var     setvar  stmt
and     op1     op2
or      op1     op2
not     op1

leafs:

single argument:
       ID IDSET idset_set NUM STRING VER WARNING ERROR FATAL_ERROR PROVIDES DEPENDS COPY_PENDING DEFINED

double argument:
       SET EQUAL NOT_EQUAL LESS GREATER MATCH

node:
        IF FOREACH AND OR NOT
*/

%token <elem> ID IDSET NUM STRING CHAR VER
%token <line>IF THEN ELSE FOREACH
%token <line>WARNING ERROR FATAL_ERROR FGREP STAT CRYPT SPLIT NUMERIC SET ADD_TO_OPT
%token <line>PROVIDES DEPENDS VERSION
%token UNKNOWN FI IN DO DONE ON SAMENET SUBNET
%left <line>ASSIGN
%right ORELSE
%left AND OR ADD SUB MULT DIV MOD
%left EQUAL NOT_EQUAL LESS GREATER LE GE MATCH
%left NOT COPY_PENDING DEFINED UNIQUE
%left CAST

%type <elem> condition id idset idset_set index num_expr versionlist
%type <elem> val statement depend provides line if_statement foreach_statement input

/* Grammar follows */
%%

lang    : input         { add_script($1); }
        ;
input:    line          { $$ = $1; }
        | input line    { $$ = add_node($1, $2); }
        ;

line:   statement
        | if_statement
        | foreach_statement
        ;

statement: WARNING STRING       { $$ = mknode(WARNING, $2, NULL, NULL, $1); }
        | ERROR STRING          { $$ = mknode(ERROR, $2, NULL, NULL, $1); }
        | FATAL_ERROR STRING    { $$ = mknode(FATAL_ERROR, $2, NULL, NULL, $1); }
        | ADD_TO_OPT STRING     { $$ = mknode(ADD_TO_OPT, $2, NULL, NULL, $1); }
        | ADD_TO_OPT STRING STRING { $$ = mknode(ADD_TO_OPT, $2, $3, NULL, $1); }
        | FGREP '(' STRING ',' STRING ')' { $$ = mknode(FGREP, $3, $5, NULL, $1); }
        | STAT '(' STRING ',' ID ')' { $$ = mknode(STAT, $3, $5, NULL, $1); }
        | CRYPT '(' id ')' { $$ = mknode(CRYPT, $3, NULL, NULL, $1); }
        | SPLIT '(' id ',' IDSET ',' CHAR ')' { $$ = mknode(SPLIT, $3, $5, $7,$1); }
        | SPLIT '(' id ',' IDSET ',' CHAR ',' NUMERIC ')' { $$ = mknode(SPLIT | SPLIT_NUMERIC, $3, $5, $7, $1); }
        | SET ID                { $$ = mknode(ASSIGN, $2, NULL, NULL,$1); }
        | SET ID ASSIGN val     { $$ = mknode(ASSIGN, $2, $4, NULL, $1); }
        | SET IDSET '[' index ']'  ASSIGN val   { $$ = mknode(ASSIGN, $2, $4, $7, $1); }
        | depend
        | provides
        ;

/* right recursion for head-tail list structure */
versionlist: VER             { $$ = mknode(VER, $1, NULL, NULL, $1->line); }
           | VER versionlist { $$ = mknode(VER, $1, $2, NULL, $1->line); }
           ;

depend:   DEPENDS ON ID { $$ = mknode(DEPENDS, $3, NULL, NULL, $1);}
        | DEPENDS ON ID VERSION versionlist { $$ = mknode(DEPENDS, $3, $5, NULL, $1);}
        ;

provides: PROVIDES ID VERSION VER { $$ = mknode(PROVIDES, $2, $4, NULL, $1);}
        ;

if_statement: IF condition THEN input FI { $$ = mknode(IF, $2, $4, NULL, $1);}
        | IF condition THEN input ELSE input FI { $$ = mknode(IF, $2,$4, $6, $1);}
        ;

foreach_statement: FOREACH ID IN idset_set DO input DONE { $$ = mknode(FOREACH, $2, $4, $6, $1);}
        | FOREACH ID IN ID DO input DONE { $$ = mknode(FOREACH, $2, $4, $6, $1);}
        ;

idset_set: idset_set IDSET { $$ = mknode(IDSET, $2, NULL, $1,   $2->line);}
        | IDSET            { $$ = mknode(IDSET, $1, NULL, NULL, $1->line);}
        ;

idset   : IDSET '[' ID ']'    { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        | IDSET '[' idset ']' { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        | IDSET '[' NUM ']'   { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        | idset '[' ID ']'    { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        | idset '[' idset ']' { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        | idset '[' NUM ']'   { $$ = mknode(IDSET, $1, $3, NULL, $1->line); }
        ;

id      : ID
        | idset
        ;

index   : id
        | NUM
        ;

val     : VER
        | num_expr
        | STRING
        | id ORELSE val          { $$ = mknode(ORELSE, $1, $3, NULL, $1->line);}
        | val CAST ID            { $$ = mknode(CAST, $1, $3, NULL, $1->line);}
        | val CAST NUMERIC       { $$ = mknode(CAST, $1, mkleaf(ID, "NUMERIC"), NULL, $1->line);}
        ;

num_expr: index
        | '(' num_expr ')'       { $$ = $2;}
        | num_expr ADD  num_expr { $$ = mknode(ADD, $1, $3, NULL, $1->line);}
        | num_expr MULT num_expr { $$ = mknode(MULT, $1, $3, NULL, $1->line);}
        | num_expr SUB  num_expr { $$ = mknode(SUB, $1, $3, NULL, $1->line);}
        | num_expr DIV  num_expr { $$ = mknode(DIV, $1, $3, NULL, $1->line);}
        | num_expr MOD  num_expr { $$ = mknode(MOD, $1, $3, NULL, $1->line);}
        | num_expr CAST ID       { $$ = mknode(CAST, $1, $3, NULL, $1->line);}
        | num_expr CAST NUMERIC  { $$ = mknode(CAST, $1, mkleaf(ID, "NUMERIC"), NULL, $1->line);}
        ;

condition: val                  { $$ = $1; }
        | val EQUAL     val     { $$ = mknode(EQUAL, $1, $3, NULL, -1);}
        | val NOT_EQUAL val     { $$ = mknode(NOT_EQUAL, $1, $3,NULL, -1);}
        | val LESS      val     { $$ = mknode(LESS, $1, $3, NULL, -1); }
        | val GREATER   val     { $$ = mknode(GREATER, $1, $3, NULL, -1);}
        | val LE        val     { $$ = mknode(LE, $1, $3, NULL, -1);   }
        | val GE        val     { $$ = mknode(GE, $1, $3, NULL, -1);   }
        | id  MATCH     STRING  { $$ = mknode(MATCH, $1, $3, NULL, -1);}
        | NOT condition         { $$ = mknode(NOT, $2, NULL, NULL, -1);}
        | COPY_PENDING '(' id ')' { $$ = mknode(COPY_PENDING, $3,NULL, NULL, -1);}
        | COPY_PENDING '(' STRING ')' { $$ = mknode(COPY_PENDING, $3,NULL, NULL, -1);}
        | UNIQUE '(' id ')' { $$ = mknode(UNIQUE, $3,NULL, NULL, -1);}
        | SAMENET '(' STRING ',' STRING ')' { $$ = mknode(SAMENET, $3, $5, NULL, -1);}
        | SUBNET '(' STRING ',' STRING ')' { $$ = mknode(SUBNET, $3, $5, NULL, -1);}
        | DEFINED '(' id ')'    { $$ = mknode(DEFINED, $3,NULL, NULL, -1);}
        | condition AND condition { $$ = mknode(AND, $1, $3, NULL, -1);}
        | condition OR condition  { $$ = mknode(OR, $1, $3, NULL, -1);}
        | '(' condition ')'     { $$ = $2;                              }
        ;
%%

int
yyerror(char const *s)  /* Called by yyparse on error */
{
    char const *p = yytext;

    printf ("(%s:%d) %s (last token: '", parse_get_current_file(), yyline, s);
    while (*p) {
        printf("%s", log_get_printable(*p));
        ++p;
    }
    printf("')\n");
    return 0;
}
