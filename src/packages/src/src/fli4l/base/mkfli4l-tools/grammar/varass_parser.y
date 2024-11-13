/*****************************************************************************
 *  @file grammar/varass_parser.y
 *  Parser for files containing variable assignments.
 *
 *  Copyright (c) 2012-2016 The fli4l team
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
%parse-param {struct varass_parse_tree_t *tree}
%parse-param {int priority}
%locations
%error-verbose

%{
#include "grammar/varass.h"
%}

%union {
    unsigned number;
    char *str;
    struct qstr_t *qstr;
    struct identifier_t *id;
    struct varass_t *assignment;
    struct varass_target_t *target;
    struct varass_list_t *asslist;
}

%destructor { free($$); } <str>
%destructor { qstr_destroy($$); } <qstr>
%destructor { identifier_destroy($$); } <id>
%destructor { varass_destroy($$); } <assignment>
%destructor { varass_target_destroy($$); } <target>
%destructor { varass_list_destroy($$); } <asslist>

%{
#include <string.h>
#include <stdlib.h>
#include "grammar/varass_scanner.h"
#include "grammar/parser.h"

PARSER_DEFINE_ERROR_PROLOGUE(struct varass_parse_tree_t *)
int priority,
PARSER_DEFINE_ERROR_EPILOGUE("variable assignments")
%}

%token <id> ID
%token EQ LBRACKET RBRACKET DOT LBRACE RBRACE
%token <qstr> QSTRING
%token <str> COMMENT
%token <str> UNKNOWN
%token <number> ABSOLUTE_INDEX

%type <str> comment
%type <assignment> assignment nested_assignment
%type <asslist> ass-list ass-block non-empty-ass-block
%type <target> target nested_target

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

/* statement which is either an assignment or an error */
statement:
  assignment  { varass_parse_tree_add_assignment(tree, $1); }
| error       { tree->ok = FALSE; }
;

/* targets */
target:
  ID {
    $$ = varass_target_create_top_level($1);
  }
| target DOT ID {
    $$ = varass_target_create_struct_member($1, $3);
  }
| target LBRACKET ABSOLUTE_INDEX RBRACKET {
    $$ = varass_target_create_array_member($1, varass_index_create_absolute($3));
  }
| target LBRACKET RBRACKET {
    $$ = varass_target_create_array_member($1, varass_index_create_next_free());
  }
;

nested_target:
  ID {
    $$ = varass_target_create_struct_member(varass_target_create_top_level(NULL), $1);
  }
| nested_target DOT ID {
    $$ = varass_target_create_struct_member($1, $3);
  }
| LBRACKET ABSOLUTE_INDEX RBRACKET {
    $$ = varass_target_create_array_member(varass_target_create_top_level(NULL), varass_index_create_absolute($2));
  }
| nested_target LBRACKET ABSOLUTE_INDEX RBRACKET {
    $$ = varass_target_create_array_member($1, varass_index_create_absolute($3));
  }
| LBRACKET RBRACKET {
    $$ = varass_target_create_array_member(varass_target_create_top_level(NULL), varass_index_create_next_free());
  }
| nested_target LBRACKET RBRACKET {
    $$ = varass_target_create_array_member($1, varass_index_create_next_free());
  }
;

/* variable assignments */
assignment:
  target EQ QSTRING comment ass-block
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = varass_create($1, $3, priority, $4, location, $5);
}
| target comment non-empty-ass-block
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = varass_create($1, NULL, priority, $2, location, $3);
}
;

nested_assignment:
  nested_target EQ QSTRING comment ass-block
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = varass_create($1, $3, priority, $4, location, $5);
}
| nested_target comment non-empty-ass-block
{
    struct location_t *location
        = location_create(file, @1.first_line, @1.first_column);
    $$ = varass_create($1, NULL, priority, $2, location, $3);
}
;

ass-block:
  /* empty */         { $$ = varass_list_create(); }
| non-empty-ass-block { $$ = $1; }
;

non-empty-ass-block:
  LBRACE ass-list RBRACE { $$ = $2; }
;

ass-list:
  /* empty */                { $$ = varass_list_create(); }
| ass-list nested_assignment { $$ = varass_list_append($1, $2);  }
;

/* comment that does not begin a line */
comment:
  /* empty */ { $$ = strdup(""); }
| COMMENT     { $$ = $1; }
;
