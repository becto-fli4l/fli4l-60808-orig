/*****************************************************************************
 *  @file grammar/expr_parser.yy
 *  Parser for expressions.
 *
 *  Copyright (c) 2015-2016 The fli4l team
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
%parse-param {struct expr_parse_tree_t *tree}
%parse-param {struct var_scope_t *scope}
%locations
%error-verbose

%{
#include "grammar/expr.h"
%}

%union {
    char *str;
    struct qstr_t *qstr;
    struct number_t *number;
    struct identifier_t *id;
    struct expression_t *expression;
}

%destructor { free($$); } <str>
%destructor { qstr_destroy($$); } <qstr>
%destructor { number_destroy($$); } <number>
%destructor { identifier_destroy($$); } <id>
%destructor { expression_destroy($$, TRUE); } <expression>

%{
#include <string.h>
#include <stdlib.h>
#include "grammar/expr_scanner.h"
#include "grammar/parser.h"

PARSER_DEFINE_ERROR_PROLOGUE(struct expr_parse_tree_t *)
struct var_scope_t *scope,
PARSER_DEFINE_ERROR_EPILOGUE("expression")
%}

%token <id> ID
%token <qstr> QSTRING
%token <number> NUMBER
%token <str> UNKNOWN

%include grammar/expr_parser.tokens

%type <expression> expression

%%

line:
  expression {
    tree->expr = $1->expr;
    tree->ok = $1->valid;
    expression_destroy($1, FALSE);
}
| error      { tree->ok = FALSE; }
;

%include grammar/expr_parser.rules
