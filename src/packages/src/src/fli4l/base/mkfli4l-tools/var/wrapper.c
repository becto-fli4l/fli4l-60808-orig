/*****************************************************************************
 *  @file var/wrapper.c
 *  Wrapper functions.
 *
 *  Copyright (c) 2000-2001 - Frank Meyer
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
 *  Last Update: $Id$
 *****************************************************************************
 */

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include "core.h"

#define MODULE "var::wrapper"

char const *
var_read(char const *name)
{
    struct var_instance_t *const inst
        = var_instance_create_from_string(var_get_global_scope(), name);

    if (inst) {
        struct var_value_t *const value = var_instance_get_value(inst);
        char const *const result = value ? value->value : NULL;
        var_value_destroy(value);
        var_instance_destroy(inst);
        return result;
    }
    else {
        return NULL;
    }
}

char const *
var_try_read(char const *name)
{
    struct var_instance_t *const inst
        = var_instance_create_from_string(var_get_global_scope(), name);

    if (inst) {
        struct var_value_t *const value = var_instance_try_get_value(inst);
        char const *const result = value ? value->value : NULL;
        var_value_destroy(value);
        var_instance_destroy(inst);
        return result;
    }
    else {
        return NULL;
    }
}

BOOL
var_write(
    char const *name,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority,
    BOOL check
)
{
    struct var_instance_t *const inst
        = var_instance_create_from_string(var_get_global_scope(), name);

    if (inst) {
        BOOL const result = check
            ? var_instance_set_and_check_value(inst, value, extra, mode, priority)
            : var_instance_set_value(inst, value, extra, mode, priority);
        var_instance_destroy(inst);
        return result;
    }
    else {
        return FALSE;
    }
}
