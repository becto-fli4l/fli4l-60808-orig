/*----------------------------------------------------------------------------
 *  var.h - functions for processing variables
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

#ifndef LIBMKFLI4L_VAR_H_
#define LIBMKFLI4L_VAR_H_

#include "defs.h"
#include "package.h"

#define MAX_FILENAMES           1024                /* max. 1024 filenames  */
#define MAX_VARIABLES           2048                /* max. 1024 variables  */
#define VAR_SIZE                128                 /* variable size        */

#define TYPE_UNKNOWN            0
#define TYPE_NUMERIC            1

typedef struct {
    BOOL requested;
    BOOL copied;
} slot_flags_t;

slot_flags_t *create_slot_flags(void);

typedef struct {
    BOOL generated;
    char *set_var;
} var_flags_t;

var_flags_t *create_var_flags(BOOL generated, char const *set_var);
void *clone_var_flags(void *data);
void free_var_flags(void *data);

int     mark_var_requested(char const *name);
int     mark_var_copied(char const *name);

BOOL    is_var_tagged(char const *name);
BOOL    is_var_numeric(char const *name);
BOOL    is_var_weak(char const *name);
BOOL    is_var_generated(char const *name);
BOOL    is_var_copy_pending(char const *name);

void    var_add_weak_declaration(char const *package, char const *name, char const *value,
                                 char const *comment, char const *set_var,
                                 int type, char const *file, int line,
                                 int log_level, int filetype);

/**
 * Dumps all enabled variables into a file. Variables with priority
 * VAR_PRIORITY_SCRIPT_TRANSIENT are excluded from the dump.
 *
 * @param fname
 *  The name of the file where the variables are to be stored.
 * @return
 *  TRUE if the dump was successful, FALSE otherwise.
 */
BOOL               dump_variables(char const *fname);
extern char const *get_variable(char const *);
extern char const *try_get_variable(char const *);
extern char const *get_variable_package(char const *name);
extern char const *get_variable_comment(char const *);
extern char *get_variable_src(char const *name);

struct iter_t;
struct iter_t * init_set_var_iteration(char const *set_var);
char *          get_next_set_var(struct iter_t *iter);
int             get_last_index(struct iter_t *iter);
void            end_set_var_iteration(struct iter_t *iter);

extern void     var_init(void);
extern void     read_config_file(struct package_file_t *file, struct package_t *package, int priority);
extern BOOL     config_process_assignments(void);
extern int      set_variable(char const *package, char const *name,
                             char const *content, char const *comment,
                             BOOL supersede, char const *config_file,
                             BOOL dq, int line);

#endif
