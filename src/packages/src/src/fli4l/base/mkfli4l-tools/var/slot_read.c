/*****************************************************************************
 *  @file var/slot_read.c
 *  Functions for reading from variable slots.
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
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "slot_read.h"

#define MODULE "var::slot_read"

unsigned
var_instance_get_next_free_index(struct var_instance_t *THIS)
{
    int level;

    union varslot_t *slot = &THIS->var->slot;
    unsigned const *index = THIS->indices;

    if (THIS->num_indices >= THIS->var->level) {
        return 0;
    }

    for (level = 0; level < THIS->num_indices; ++level, ++index) {
        if (*index >= slot->array.num_slots) {
            return 0;
        }

        slot = &slot->array.slots[*index];
    }

    return slot->array.num_slots;
}

union varslot_t *
var_instance_get_slot_for_reading(struct var_instance_t *THIS, BOOL warn)
{
    int level;

    union varslot_t *slot = &THIS->var->slot;
    unsigned const *index = THIS->indices;

    if (THIS->num_indices > THIS->var->level) {
        char *inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Excessive indices (%d instead of %d) while addressing variable '%s' for reading.\n",
            THIS->num_indices,
            THIS->var->level,
            inst_name
        );
        free(inst_name);
        return NULL;
    }

    for (level = 0; level < THIS->num_indices; ++level, ++index) {
        if (*index >= slot->array.num_slots) {
            if (warn) {
                char *const inst_name = var_instance_to_string(THIS);
                log_error(
                    MODULE ": Index '%u' of dimension %d out of bounds (maximum allowed index is %u) while reading variable '%s'.\n",
                    *index + 1,
                    level + 1,
                    slot->array.num_slots,
                    inst_name
                );
                free(inst_name);
            }
            return NULL;
        }

        slot = &slot->array.slots[*index];
    }

    return slot;
}

/**
 * Checks indices against array bounds. This is necessary as the array bounds
 * may have been set to fixed values that are lower than the real array
 * dimensions.
 *
 * @param THIS
 *  The variable instance addressing the array entry.
 * @param warn
 *  If TRUE, warn about indices which are out of bounds.
 * @return
 *  TRUE if the check was successful, FALSE otherwise.
 */
static BOOL
var_check_array_bounds(struct var_instance_t *THIS, BOOL warn)
{
    int bound = 0;
    int const lvl_index = THIS->var->level - 1;
    struct var_t *const var_n = THIS->var->var_n;

    struct var_instance_t *inst_n
        = var_instance_create(var_n, var_n->level, THIS->indices);
    struct var_value_t *value = var_instance_try_get_value(inst_n);
    if (value) {
        if (!var_value_convert_to_integer(value, &bound)) {
            char *const inst_name_var_n = var_instance_to_string(inst_n);
            char *const inst_name = var_instance_to_string(THIS);
            log_error(
                MODULE ": Failed to convert value '%s' of array bound variable '%s' to an integer value while accessing variable '%s'.\n",
                value->value,
                inst_name_var_n,
                inst_name
            );
            free(inst_name);
            free(inst_name_var_n);
            var_value_destroy(value);
            var_instance_destroy(inst_n);
            return FALSE;
        }
        else {
            var_value_destroy(value);
        }
    }

    if ((int) THIS->indices[lvl_index] < bound) {
        var_instance_destroy(inst_n);
        return TRUE;
    }
    else {
        if (warn) {
            char *const inst_name_var_n = var_instance_to_string(inst_n);
            char *const inst_name = var_instance_to_string(THIS);
            log_info(
                LOG_VAR,
                MODULE ": Level %d index %d not less than array bound %d defined by array bound variable '%s' while accessing variable '%s'.\n",
                lvl_index,
                THIS->indices[lvl_index],
                bound,
                inst_name_var_n,
                inst_name
            );
            free(inst_name);
            free(inst_name_var_n);
        }

        var_instance_destroy(inst_n);
        return FALSE;
    }
}

/**
 * Returns the value of a variable slot.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot. Excessive or missing
 *  indices are flagged as an error.
 * @param warn
 *  If TRUE, warn about indices which are out of bounds.
 * @return
 *  The value of the variable or NULL if the accessed variable slot has not
 *  been set. Use var_value_destroy() to destroy it if you don't need it
 *  anymore.
 */
static struct var_value_t *
var_instance_do_get_value(struct var_instance_t *THIS, BOOL warn)
{
    union varslot_t *slot;

    if (THIS->num_indices < THIS->var->level) {
        char *inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Missing indices (%d instead of %d) while reading variable '%s'.\n",
            THIS->num_indices,
            THIS->var->level,
            inst_name
        );
        free(inst_name);
        return NULL;
    }

    slot = var_instance_get_slot_for_reading (THIS, warn);
    if (slot) {
        struct var_value_t *result = var_instance_read_value(THIS, slot, warn);
        if (!result || result->value != NULL) {
            return result;
        }
        else {
            var_value_destroy(result);
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

struct var_value_t *
var_instance_read_value(
    struct var_instance_t *THIS,
    union varslot_t *slot,
    BOOL warn
)
{
    struct var_value_t *result;
    char *inst_name;
    BOOL enabled;

    if (THIS->var->level > 0 && !var_check_array_bounds(THIS, warn)) {
        return NULL;
    }

    inst_name = var_instance_to_string(THIS);
    if (slot->entry.read_active) {
        log_error(
            MODULE ": Dependency cycle detected while reading variable '%s'.\n",
            inst_name
        );
        free(inst_name);
        return NULL;
    }

    slot->entry.read_active = TRUE;
    enabled = var_instance_is_enabled(THIS);
    result = var_slot_get_value(slot, THIS);
    slot->entry.read_active = FALSE;

    if (enabled) {
        if (result->kind == VAR_VALUE_KIND_NONE) {
            if (warn) {
                log_error(MODULE ": Reading unset variable '%s'.\n", inst_name);
            }
            else {
                log_info(
                    LOG_VAR,
                    MODULE ": Reading unset variable '%s'.\n",
                    inst_name
                );
            }
        }
        else {
            log_info(
                LOG_VAR,
                MODULE ": Reading variable '%s' yields '%s' (priority %d).\n",
                inst_name,
                result->value,
                result->priority
            );
        }
    }
    else {
        result->kind = VAR_VALUE_KIND_DISABLED;

        if (THIS->var->props->dis_value) {
            result->value = THIS->var->props->dis_value;

            log_info(
                LOG_VAR,
                MODULE ": Reading disabled variable '%s' yields '%s'.\n",
                inst_name,
                result->value
            );
        }
        else {
            result->value = NULL;
            if (warn) {
                log_error(MODULE ": Reading disabled variable '%s'.\n", inst_name);
            }
        }
    }

    free(inst_name);
    return result;
}

struct var_value_t *
var_instance_get_value(struct var_instance_t *THIS)
{
    return var_instance_do_get_value(THIS, TRUE);
}

struct var_value_t *
var_instance_try_get_value(struct var_instance_t *THIS)
{
    return var_instance_do_get_value(THIS, FALSE);
}
