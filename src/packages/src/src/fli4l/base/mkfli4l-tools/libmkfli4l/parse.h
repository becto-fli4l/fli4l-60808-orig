/*----------------------------------------------------------------------------
 *  parse.h - functions for parsing
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
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

#ifndef LIBMKFLI4L_PARSE_H_
#define LIBMKFLI4L_PARSE_H_

#include <stdio.h>
#include "defs.h"
#include "array.h"
#include "package.h"
#include "tree.h"

#ifdef DEBUG
extern int yy_flex_debug;
extern int yydebug;
#endif
extern int yyline;

struct opt_t {
    char *name;
    char *value;
};

#define CMP_EQUAL    0
#define CMP_LESS    -1
#define CMP_GREATER  1

BOOL str_strmatch(char const *value, char const *expr, char const *package, char const *file, int line);
int val_numcmp(char const *var, char const *content, char const *package, int line);
int val_vercmp(char const *var, char const *version, char const *package, int line);

char const *parse_get_variable(char const *name, char const *package, int line);
char *parse_rewrite_string(char const *name, char const *package, int line);

void parse_warning(char const *warning, char const *package, int line);
void parse_error(char const *warning, char const *package, int line);
void parse_fatal_error(char const *warning, char const *package, int line);
void parse_add_to_opt(char const *file, char const *options, char const *package, int line);
void parse_stat(char const *file, char const *id, char const *package, char const *pkgfile, int line);
void parse_fgrep(char const *file, char const *search, char const *package, char const *pkgfile, int line);
void parse_crypt(char const *id, char const *package, char const *pkgfile, int line);
void parse_provides(char const *id, char const *version, char const *package, char const *pkgfile, int line);
void parse_depends(char const *id, elem_t *version, char const *package,
                   char const *pkgfile, int line);
int parse_copy_pending (char const *name);

char const *parse_get_current_package(void);
char const *parse_get_current_file(void);

/**
 * Parses an extended check script.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The extended check script to be parsed.
 * @return
 *  TRUE if parsing the extended check script was successful, FALSE
 *  otherwise.
 */
BOOL
parse_check_file(struct package_file_t *file, struct package_t *package);

/**
 * Parses and executes all extended check scripts.
 * @return
 *  OK on success, ERR otherwise.
 */
BOOL
execute_all_extended_checks(void);

int yyparse (void);
array_t *parse_entry_options(char const *opts);

int add_to_zip_list(char const *var, char const *content, char const *filename, array_t *opt, char const *opt_file, int line);

#endif
