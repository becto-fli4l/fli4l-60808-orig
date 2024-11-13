/*****************************************************************************
 *  @file grammar/vartype_parser.yy
 *  Parser for files containing variable types.
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
%parse-param {struct vartype_parse_tree_t *tree}
%locations
%error-verbose

%{
#include "grammar/vartype.h"
%}

%union {
    char *str;
    struct qstr_t *qstr;
    struct number_t *number;
    struct identifier_t *id;
    struct vartype_def_t *def;
    struct vartype_ext_t *ext;
    struct expression_t *expression;
}

%destructor { free($$); } <str>
%destructor { qstr_destroy($$); } <qstr>
%destructor { number_destroy($$); } <number>
%destructor { identifier_destroy($$); } <id>
%destructor { vartype_def_destroy($$); } <def>
%destructor { vartype_ext_destroy($$); } <ext>
%destructor { expression_destroy($$, TRUE); } <expression>

%{
#include <string.h>
#include <stdlib.h>
#include "grammar/vartype_scanner.h"
#include "grammar/parser.h"

PARSER_DEFINE_ERROR_PROLOGUE(struct vartype_parse_tree_t *)
PARSER_DEFINE_ERROR_EPILOGUE("variable types")

#define scope var_get_global_scope()
%}

%token <id> ID
%token EQ
%token COLON
%token EXT
%token <qstr> QSTRING
%token <number> NUMBER
%token <str> COMMENT
%token <str> UNKNOWN

%include grammar/expr_parser.tokens

%type <str> comment
%type <def> definition
%type <ext> extension cond_extension
%type <expression> expression

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

/* statement which is either a definition, an extension, or an error */
statement:
  definition         { vartype_parse_tree_add_definition(tree, $1); }
| extension          { vartype_parse_tree_add_extension(tree, $1); }
| cond_extension     { vartype_parse_tree_add_extension(tree, $1); }
| error              { tree->ok = FALSE; }
;

/* type definition */
definition:
  ID EQ QSTRING COLON QSTRING comment
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = vartype_def_create($1, $3, $5, $6, location);
}
;

/* type extension without a condition */
extension:
  EXT ID EQ QSTRING COLON QSTRING comment
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = vartype_ext_create($2, NULL, $4, $6, $7, location);
}
;

/* type extension with a condition */
cond_extension:
  EXT ID LBRACKET expression RBRACKET EQ QSTRING COLON QSTRING comment
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = vartype_ext_create($2, $4, $7, $9, $10, location);
}
;

/* comment that does not begin a line */
comment:
  /* empty */ { $$ = strdup(""); }
| COMMENT     { $$ = $1; }
;

%include grammar/expr_parser.rules
