/*----------------------------------------------------------------------------
 *  regex.h - functions for processing regular expressions
 *
 *  Copyright (c) 2002-2005 - Jean Wolter
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
 *  Creation:    2002-03-01  jw5
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#ifndef LIBMKFLI4L_REGEX_H_
#define LIBMKFLI4L_REGEX_H_

#include <regex.h>
#include "defs.h"
#include "package.h"

#define REGEX_MATCHED 0
#define REGEX_NOT_MATCHED 1
#define REGEX_COMPILE_ERROR 2

int regexp_chkvar(char const *name, char const *value,
                  char const *regexp_name, char const *user_regexp);
int regexp_user(char const *value, char const *expr, size_t nmatch, regmatch_t * pmatch,
                BOOL modify_expr, char const *file, int line);
int regexp_exists(char const *name);
int regexp_find_type(char const *regexp_name);

void regexp_init(void);
void regexp_read_file(struct package_file_t *file, struct package_t *package);
BOOL regexp_process_definitions(void);
BOOL regexp_process_extensions(void);
void regexp_done(void);

#endif
