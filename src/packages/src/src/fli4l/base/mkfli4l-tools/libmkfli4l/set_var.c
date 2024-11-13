/*----------------------------------------------------------------------------
 *  set_var.c - functions for processing set-like variables
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

#include <var.h>
#include "libmkfli4l/check.h"
#include "libmkfli4l/var.h"
#include "libmkfli4l/log.h"
#include "libmkfli4l/str.h"
#include "libmkfli4l/array.h"
#include <stdlib.h>
#include <string.h>

struct iter_t {
    struct var_value_iterator_t *iter;
};

int
get_last_index(struct iter_t *iter)
{
    struct var_instance_t *inst = var_value_iterator_get_var_instance(iter->iter);
    if (inst->num_indices == 0) {
        return 0;
    }
    else {
        return inst->indices[inst->num_indices - 1] + 1;
    }
}

void
end_set_var_iteration(struct iter_t *iter)
{
    var_value_iterator_destroy(iter->iter);
    free(iter);
}

struct iter_t *
init_set_var_iteration(char const *set_var)
{
    struct var_t *var;
    struct var_instance_t *inst;
    struct iter_t *iter;

    var = var_try_get(set_var);
    if (!var) {
        log_error("Error: variable '%s' does not exist\n", set_var);
        return NULL;
    }

    inst = var_instance_create(var, 0, NULL);
    if (!inst) {
        log_error("Error creating variable instance for '%s'", set_var);
        return NULL;
    }

    iter = (struct iter_t *) safe_malloc(sizeof(struct iter_t));
    iter->iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    return iter;
}

char *
get_next_set_var(struct iter_t *iter)
{
    if (var_value_iterator_next(iter->iter)) {
        return var_instance_to_string(var_value_iterator_get_var_instance(iter->iter));
    }
    else {
        return NULL;
    }
}
