/*****************************************************************************
 *  @file var/slot_check.c
 *  Functions for checking variable slots.
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
#include <container/hashmap.h>
#include <expression.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "slot.h"
#include "slot_read.h"
#include "scope.h"

#define MODULE "var::slot_check"

/**
 * Checks a single non-array slot.
 *
 * @param value
 *  The value to check.
 * @param inst
 *  The variable instance this value belongs to.
 * @return
 *  TRUE if the value has been successfully checked, FALSE otherwise.
 */
static BOOL
var_check_value(struct var_value_t *value, struct var_instance_t *inst)
{
    char *const inst_name = var_instance_to_string(inst);
    BOOL result = FALSE;

    if (value->value == NULL) {
        if (value->kind == VAR_VALUE_KIND_DISABLED) {
            log_info(
                LOG_VAR,
                MODULE ": Variable '%s' not enabled, skipping check.\n",
                inst_name
            );
            result = TRUE;
        }
        else if (!inst->var->props->optional) {
            log_error(
                MODULE ": Value of mandatory variable '%s' has not been set.\n",
                inst_name
            );
        }
        else {
            log_info(
                LOG_VAR,
                MODULE ": Value of optional variable '%s' is missing, skipping type check.\n",
                inst_name
            );
            result = TRUE;
        }
    }
    else {
        if (vartype_match(inst->var->type, value->value)) {
            log_info(
                LOG_VAR,
                MODULE ": Value '%s' of variable '%s' successfully checked.\n",
                value->value,
                inst_name
            );
            result = TRUE;
        }
        else {
            char const *errmsg = vartype_get_complete_error_message(inst->var->type);
            if (*errmsg) {
                log_error(
                    MODULE ": Value '%s' of variable '%s' is not properly typed: %s\n",
                    value->value,
                    inst_name,
                    errmsg
                );
            }
            else {
                log_error(
                    MODULE ": Value '%s' of variable '%s' is not properly typed.\n",
                    value->value,
                    inst_name
                );
            }
        }
    }

    free(inst_name);
    return result;
}

static BOOL
var_check_variable(void *entry, void *data)
{
    struct var_t *var = (struct var_t *) entry;
    struct var_instance_t *inst = var_instance_create(var, 0, NULL);
    struct var_value_iterator_t *iter = var_value_iterator_create(inst, TRUE);
    BOOL result = TRUE;

    log_info(
        LOG_VAR,
        MODULE ": Checking %s.\n",
        var_get_name(var)
    );

    while (var_value_iterator_next(iter)) {
        struct var_value_t *const value = var_value_iterator_get(iter);
        if (!var_check_value(value, var_value_iterator_get_var_instance(iter))) {
            result = FALSE;
        }
    }

    var_value_iterator_destroy(iter);
    var_instance_destroy(inst);
    if (!result) {
        *(BOOL *) data = FALSE;
    }

    return TRUE;
}

BOOL
var_check_values(void)
{
    BOOL result = TRUE;

    log_info(
        LOG_VAR,
        MODULE ": About to check the values of all variables.\n"
    );

    var_scope_foreach(var_get_global_scope(), TRUE, &var_check_variable, &result);
    return result;
}
