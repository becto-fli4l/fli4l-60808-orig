/*****************************************************************************
 *  @file grammar/vardef_parser.yy
 *  Parser for files containing variable definitions.
 *
 *  Copyright (c) 2012-2015 The fli4l team
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
 *****************************************************************************
 */

%pure-parser
%lex-param   {void *scanner}
%parse-param {void *scanner}
%parse-param {struct package_file_t *file}
%parse-param {struct vardef_parse_tree_t *tree}
%locations
%error-verbose

%{
#include "grammar/vardef.h"
%}

%union {
    BOOL bool_value;
    char *str;
    struct qstr_t *qstr;
    struct number_t *number;
    struct identifier_t *id;
    struct vardef_t *def;
    struct vardef_template_t *tmpl_def;
    struct vardef_typeref_t *type;
    struct expression_t *condition;
    struct expression_t *defvalue;
    struct identifier_t *arraybound;
}

%destructor { free($$); } <str>
%destructor { qstr_destroy($$); } <qstr>
%destructor { number_destroy($$); } <number>
%destructor { identifier_destroy($$); } <id>
%destructor { vardef_destroy($$); } <def>
%destructor { vardef_destroy_template($$); } <tmpl_def>
%destructor { vardef_typeref_destroy($$); } <type>
%destructor { expression_destroy($$, TRUE); } <condition>

%{
#include <string.h>
#include <stdlib.h>
#include "grammar/vardef_scanner.h"
#include "grammar/parser.h"

PARSER_DEFINE_ERROR_PROLOGUE(struct vardef_parse_tree_t *)
PARSER_DEFINE_ERROR_EPILOGUE("variable definitions")

#define scope var_get_global_scope()
%}

%token NEWLINE
%token <id> ID
%token <id> TEMPLATE_ID
%token OPT
%token NONE
%token <qstr> QSTRING
%token <number> NUMBER
%token <str> REGEX
%token <str> COMMENT
%token <str> UNKNOWN

%include grammar/expr_parser.tokens

%type <bool_value> opt
%type <str> comment
%type <defvalue> defvalue
%type <def> definition
%type <tmpl_def> template_definition
%type <condition> condition
%type <condition> expression
%type <arraybound> arraybound
%type <type> type

%%

/* file which contains a list of statements */
file:
  statements
;

/* statement list which may be empty */
statements:
  /* empty */
| statements statement
;

/* statement which is either a definition or an error */
statement:
  definition NEWLINE          { vardef_parse_tree_add_definition(tree, $1); }
| template_definition NEWLINE { vardef_parse_tree_add_template_definition(tree, $1); }
| definition error            { $1->valid = FALSE; vardef_parse_tree_add_definition(tree, $1); }
| template_definition error   { $1->valid = FALSE; vardef_parse_tree_add_template_definition(tree, $1); }
| error                       { tree->ok = FALSE; }
;

/* variable definition */
definition:
  opt ID condition arraybound type defvalue comment
{
    struct location_t *location = location_create(
        file,
        $1 ? @1.first_line : @2.first_line,
        $1 ? @1.first_column : @2.first_column
    );
    $$ = vardef_create($1, $2, $3, $4, $5, $6, $7, location);
}
;

/* variable template definition */
template_definition:
  opt TEMPLATE_ID condition arraybound type defvalue comment
{
    struct location_t *location = location_create(
        file,
        $1 ? @1.first_line : @2.first_line,
        $1 ? @1.first_column : @2.first_column
    );
    $$ = vardef_create_template($1, $2, $3, $4, $5, $6, $7, location);
}
;

/* optionality marker */
opt:
  /* empty*/  { $$ = FALSE; }
| OPT         { $$ = TRUE; }
| OPT OPT     { $$ = TRUE; }
;

/* condition represented by an arbitrary boolean expression */
condition:
  expression  { $$ = $1; }
| NONE        { $$ = NULL; }
;

/* explicit array bound variable; now obsolete and ignored */
arraybound:
  ID          { $$ = $1; }
| NONE        { $$ = NULL; }
;

/* type represented either by an identifier or by a regular expression */
type:
  ID          { $$ = vardef_typeref_create_named($1); }
| REGEX       { $$ = vardef_typeref_create_unnamed($1); }
;

/* default value (may be missing) */
defvalue:
  /* empty */ { $$ = NULL; }
| expression  { $$ = $1; }
;

/* comment that does not begin a line */
comment:
  /* empty */ { $$ = strdup(""); }
| COMMENT     { $$ = $1; }
;

%include grammar/expr_parser.rules
