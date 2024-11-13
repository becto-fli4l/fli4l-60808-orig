/*----------------------------------------------------------------------------
 *  check.h - functions for checking variables
 *
 *  Copyright (c) 2001 - Frank Meyer
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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
 *  Creation:    2001-08-12  fm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#ifndef LIBMKFLI4L_CHECK_H_
#define LIBMKFLI4L_CHECK_H_

#include "defs.h"
#include "package.h"

extern void check_init(void);
extern void read_check_file(struct package_file_t *file, struct package_t *package);
extern BOOL check_process_definitions(void);
extern BOOL check_process_template_definitions(void);
extern char *get_set_var_n(char const *);
extern BOOL check_var_defined(char const *var);
extern BOOL check_var_numeric(char const *var);
extern BOOL check_var_optional(char const *var);
extern int check_all_variables(void);

struct var_t;
struct var_t * check_add_weak_declaration(char const *package, char const *name,
                                          int type, BOOL remove,
                                          char const *file, int line,
                                          int log_level);

#endif
