/*****************************************************************************
 *  @file var/slot_write.c
 *  Functions for writing into variable slots.
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
#include <errno.h>
#include "core.h"
#include "slot_write.h"

#define MODULE "var::slot_write"

/**
 * Resizes an array slot. The caller must ensure that this function is only
 * called on array slots, i.e. on slots with a level smaller than the
 * variable's level.
 *
 * @param THIS
 *  The slot to resize.
 * @param level
 *  The slot level.
 * @param maxlevel
 *  The variable level.
 * @param tot_num_slots
 *  The new total number of entries in this slot.
 */
static void
var_slot_resize(
    union varslot_t *THIS,
    int level,
    int maxlevel,
    unsigned tot_num_slots
)
{
    int const next_level = level + 1;
    unsigned const add_num_slots = tot_num_slots - THIS->array.num_slots;

    union varslot_t *new_slot;
    unsigned i;

    THIS->array.slots = (union varslot_t *) safe_realloc(
        THIS->array.slots, tot_num_slots * sizeof(union varslot_t)
    );

    for (new_slot = THIS->array.slots + THIS->array.num_slots, i = 0;
            i < add_num_slots; ++i, ++new_slot) {
        var_slot_init(new_slot, next_level, maxlevel);
    }

    THIS->array.num_slots = tot_num_slots;
}

/**
 * Expands the array slots of all clients of some variable, given an array of
 * indices. The expansion is performed by calling var_slot_expand() on each
 * client.
 * 
 * @param var
 *  The variable whose clients are to be processed.
 * @param num_indices
 *  The number of indices to use.
 * @param indices
 *  The indices to use.
 */
static void
var_slot_expand_clients(struct var_t *var, int num_indices, unsigned *indices);

/**
 * Expands the array slots of this variable instance if necessary. In addition,
 * this expansion request is delegated to all clients of the underlying
 * variable by calling var_slot_expand_clients(), such that it properly
 * propagates downwards.
 * 
 * @param THIS
 *  The variable instance addressing the array slot.
 */
static void
var_slot_expand(struct var_instance_t *THIS)
{
    int level;
    unsigned const *index;
    union varslot_t *slot;

    index = THIS->indices;
    slot = &THIS->var->slot;
    for (
        level = 0;
        level < MIN(THIS->num_indices, THIS->var->level);
        ++level, ++index
    ) {
        if (*index >= slot->array.num_slots) {
            var_slot_resize(slot, level, THIS->var->level, (*index) + 1);
            var_slot_expand_clients(THIS->var, THIS->num_indices, THIS->indices);
        }
        slot = &slot->array.slots[*index];
    }
}

static void
var_slot_expand_clients(struct var_t *var, int num_indices, unsigned *indices) {
    struct var_t **v = var->var_n_clients;
    int i;

    for (i = 0; i < var->num_var_n_clients; ++i, ++v) {
        struct var_instance_t *inst
            = var_instance_create(*v, num_indices, indices);
        var_slot_expand(inst);
        var_instance_destroy(inst);
    }
}

/**
 * Returns the array slot given a slot address. If that slot does not exist,
 * the array is automatically resized.
 *
 * @param THIS
 *  The variable instance addressing the array slot.
 * @return
 *  The addressed slot. This slot is always valid and never NULL.
 */
static union varslot_t *
var_slot_get_or_create(struct var_instance_t *THIS)
{
    int level;
    unsigned const *index;
    union varslot_t *slot;
    struct var_t *var_n;
    struct var_t **var_n_array;

    var_n_array = (struct var_t **)
            safe_malloc(THIS->var->level * sizeof(struct var_t *));
    var_n = THIS->var->var_n;
    for (level = 0; level < THIS->var->level; ++level) {
        var_n_array[THIS->var->level - level - 1] = var_n;
        var_n = var_n->var_n;
    }

    index = THIS->indices;
    slot = &THIS->var->slot;
    for (
        level = 0;
        level < MIN(THIS->num_indices, THIS->var->level);
        ++level, ++index
    ) {
        if (*index >= slot->array.num_slots) {
            var_slot_resize(slot, level, THIS->var->level, (*index) + 1);
            var_slot_expand_clients(var_n_array[level], level + 1, THIS->indices);
        }
        slot = &slot->array.slots[*index];
    }

    free(var_n_array);
    return slot;
}

/**
 * Returns the priority of a slot. For an entry slot, the priority returned
 * corresponds to the priority stored in the entry. For an array slot, the
 * priority returned is the lowest upper bound of the priority values of all
 * subslots.
 *
 * @param THIS
 *  The slot.
 * @param level
 *  The slot level.
 * @param maxlevel
 *  The variable level.
 * @return
 *  The priority of the slot.
 */
static enum var_priority_t
var_slot_get_priority(union varslot_t const *THIS, int level, int maxlevel)
{
    if (level == maxlevel) {
        return THIS->entry.priority;
    }
    else {
        int const next_level = level + 1;
        enum var_priority_t priority = VAR_PRIORITY_UNDEFINED;

        unsigned i;
        for (i = 0; i < THIS->array.num_slots; ++i) {
            enum var_priority_t const subprio = var_slot_get_priority(
                &THIS->array.slots[i], next_level, maxlevel
            );
            if (subprio > priority) {
                priority = subprio;
            }
        }

        return priority;
    }
}

/**
 * Updates array bounds if necessary when accessing a slot.
 *
 * @param THIS
 *  The variable instance addressing the array entry.
 * @param var_n
 *  The corresponding array bound variable.
 * @param new_bound
 *  The new array bound.
 * @return
 *  TRUE if the update was successful, FALSE otherwise. The latter can happen
 *  if the array bound has been set explicitly by the user.
 */
static BOOL
var_update_array_bound(struct var_instance_t *THIS, struct var_t *var_n, int new_bound)
{
    int old_bound = 0;
    BOOL result = TRUE;

    struct var_instance_t *inst_n
        = var_instance_create(var_n, var_n->level, THIS->indices);
    struct var_value_t *value = var_instance_try_get_value(inst_n);
    if (value) {
        if (!var_value_convert_to_integer(value, &old_bound)) {
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
            var_instance_destroy(inst_n);
            var_value_destroy(value);
            return FALSE;
        }
        else {
            var_value_destroy(value);
        }
    }

    if (old_bound < new_bound) {
        char *str = safe_sprintf("%d", new_bound);
        result = var_instance_do_set_value(
            inst_n, str, NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_ARRAY_BOUND,
            TRUE, TRUE
        );
        my_assert(result);
        free(str);

        value = var_instance_try_get_value(inst_n);
        if (value) {
            result = var_value_convert_to_integer(value, &old_bound);
            my_assert(result);
            var_value_destroy(value);
        }
        else {
            old_bound = 0;
        }

        result = old_bound == new_bound;
    }

    var_instance_destroy(inst_n);
    return result;
}

/**
 * Returns a pointer to some variable slot for writing. If the indices are out
 * of bounds, the array(s) are resized such that the indices lie within the
 * array bounds again. This function also updates the array bound variables
 * when some (sub)array is resized.
 *
 * @param THIS
 *  The variable instance addressing the slot. Excessive indices are flagged as
 *  an error, missing indices are allowed to address an array slot.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set.
 * @param access_failed
 *  Points to a Boolean variable which is set to TRUE on exit if functions
 *  fails because access mode and priority do not allow the slot to be changed.
 * @param ignore_base_priority
 *  If TRUE, the access priority is not checked against the variable's base
 *  priority. This is used while accessing array bound variables.
 * @return
 *  The variable slot or NULL if an error occurred (e.g. if updating the array
 *  bound variables after resizing failed). NULL is also returned if access
 *  is rejected; to distinguish this case from an error condition, you have to
 *  check *access_failed.
 */
static union varslot_t *
var_instance_get_slot_for_writing(
    struct var_instance_t *THIS,
    enum var_access_mode_t mode,
    enum var_priority_t priority,
    BOOL ignore_base_priority,
    BOOL *access_failed
)
{
    int slot_priority;
    char const *mode_str = NULL;
    BOOL ok = FALSE;

    union varslot_t *slot;
    *access_failed = FALSE;

    if (!ignore_base_priority && THIS->var->priority > priority) {
        char *inst_name = var_instance_to_string(THIS);
        if (mode != VAR_ACCESS_TENTATIVE) {
            log_error(
                MODULE ": Priority %d too low while addressing variable '%s' (priority %d) for writing.\n",
                priority,
                inst_name,
                THIS->var->priority
            );
        }
        else {
            log_info(
                LOG_VAR,
                MODULE ": Priority %d too low while addressing variable '%s' (priority %d) for writing, ignoring.\n",
                priority,
                inst_name,
                THIS->var->priority
            );
        }
        free(inst_name);
        *access_failed = TRUE;
        return NULL;
    }

    if (THIS->num_indices > THIS->var->level) {
        char *inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Excessive indices (%d instead of %d) while addressing variable '%s' for writing.\n",
            THIS->num_indices,
            THIS->var->level,
            inst_name
        );
        free(inst_name);
        return NULL;
    }

    /*
     * Okay, this is a little tricky. How do we allocate enough space for an
     * array? And how do we ensure that all arrays sharing the same array bound
     * are extended in the same way? Four functions play together to achieve
     * this: var_instance_do_set_value(), var_instance_get_slot_for_writing().
     * var_update_array_bound(), and var_slot_get_or_create(). Let us analyse
     * how this really works. For this, let us assume the existence of the
     * arrays V_%_%_A and V_%_%_B. The (implicitly or explicitly) created array
     * bound variables are V_N and V_%_N, the second of which is itself an
     * array.
     *
     * Now someone writes V_3_5_A='x'. What happens?
     * var_instance_do_set_value(V_3_5, 'x')
     * -> var_instance_get_slot_for_writing(V_3_5_A)
     *    -> var_update_array_bound(V_3_5_A)
     *       -> var_instance_do_set_value(V_3_N, 5)
     *          -> var_instance_get_slot_for_writing(V_3_N)
     *             -> var_update_array_bound(V_3_N)
     *                -> var_instance_do_set_value(V_N, 3)
     *                   -> var_instance_get_slot_for_writing(V_N)
     *                   -> V_N:=3
     *
     * Here, we set the value of V_N as usual. But we also notice that V_N has
     * clients (namely V_%_N). So:
     *
     *                      -> var_slot_get_or_create(V_3_N)
     *                         -> (extend array of V_%_N to dimensions (3))
     *
     * Then we come back and the algorithm continues similarly:
     *
     *             -> var_slot_get_or_create(V_3_N) [see below]
     *          -> V_3_N:=5
     *             -> var_slot_get_or_create(V_3_5_A)
     *                (extend array of V_%_%_A to dimensions (3,5))
     *             -> var_slot_get_or_create(V_3_5_B)
     *                (extend array of V_%_%_B to dimensions (3,5))
     *    -> var_slot_get_or_create(V_3_5_A) [see below]
     *    -> V_3_5_A:='x'
     *
     * Now the calls to var_slot_get_or_create() that are marked with [see
     * below] seem to be redundant. In an ideal world, this would be true. But
     * it can happen that the array bound variables contain "garbage", e.g.
     * unchecked values which do not represent numbers. In this case,
     * var_update_array_bound() cannot do anything useful, and the array
     * resizing of clients is not triggered either. However, the array slot
     * needs to exist anyway. So in this case, only the array the user actually
     * writes to is resized to encompass the new value; other arrays sharing
     * the array bound variables are not resized. This is not a problem,
     * however, as we strike for equally-sized arrays only within valid array
     * bounds, and this is guaranteed by the algorithm above.
     *
     * Note that there is also a technical reason for var_slot_get_or_create()
     * to be called (and which is "coded" in the name of this very function):
     * It returns the real slot to write to. Even if array resizing took place
     * as expected, we somehow need to iterate to the slot in question, and
     * this does var_slot_get_or_create() for us.
     *
     * Finally, observe that V_%_%_B is extended properly, but that does not
     * mean that V_%_%_B is now *valid* everywhere. In fact, if V_%_%_B does
     * not define a default value, this will not be the case. So the caller
     * needs to call var_check_values() eventually, in order to sort out these
     * "sparse" arrays.
     */

    if (THIS->num_indices > 0) {
        int const lvl_index = THIS->num_indices - 1;
        int const new_bound = THIS->indices[lvl_index] + 1;
        struct var_t *const var_n = THIS->var->var_n;
        if (!var_update_array_bound(THIS, var_n, new_bound)) {
            char *inst_name = var_instance_to_string(THIS);
            log_info(
                LOG_VAR,
                MODULE ": Updating array bound variable '%s' to %d failed while addressing variable '%s' (priority %d) for writing.\n",
                var_get_name(var_n),
                new_bound,
                inst_name,
                THIS->var->priority
            );
            free(inst_name);
        }
    }

    slot = var_slot_get_or_create(THIS);
    slot_priority
        = var_slot_get_priority(slot, THIS->num_indices, THIS->var->level);

    switch (mode) {
    case VAR_ACCESS_STRONG :
        mode_str = "strong";
        ok = priority > slot_priority;
        break;
    case VAR_ACCESS_WEAK :
        mode_str = "weak";
        ok = priority >= slot_priority;
        break;
    case VAR_ACCESS_TENTATIVE :
        mode_str = "tentative";
        ok = priority >= slot_priority;
        break;
    default :
        my_assert(FALSE);
    }

    if (ok) {
        return slot;
    }
    else {
        char *const inst_name = var_instance_to_string(THIS);
        if (mode != VAR_ACCESS_TENTATIVE) {
            log_error(
                MODULE ": Cannot write to '%s' (priority %d) with priority %d by %s access.\n",
                inst_name, slot_priority, priority, mode_str
            );
        }
        else {
            log_info(
                LOG_VAR,
                MODULE ": Ignoring write request to '%s' (priority %d) with priority %d by %s access.\n",
                inst_name, slot_priority, priority, mode_str
            );
        }
        free(inst_name);
        *access_failed = TRUE;
        return NULL;
    }
}

BOOL
var_instance_do_set_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority,
    BOOL check,
    BOOL ignore_base_priority
)
{
    union varslot_t *slot;
    char *inst_name;
    BOOL access_failed = FALSE;

    if (THIS->num_indices < THIS->var->level) {
        inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Missing indices (%d instead of %d) while setting variable '%s'.\n",
            THIS->num_indices,
            THIS->var->level,
            inst_name
        );
        free(inst_name);
        return FALSE;
    }

    slot = var_instance_get_slot_for_writing(
        THIS, mode, priority, ignore_base_priority, &access_failed
    );
    if (!slot) {
        if (access_failed && mode == VAR_ACCESS_TENTATIVE) {
            if (extra) {
                var_slot_extra_destroy(extra);
            }
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    inst_name = var_instance_to_string(THIS);
    if (check) { /* caller guarantees (check == TRUE) => (value != NULL) */
        log_info(
            LOG_VAR,
            MODULE ": About to check value '%s' against type of variable '%s'.\n",
            value,
            inst_name
        );

        if (vartype_match(THIS->var->type, value)) {
            log_info(
                LOG_VAR,
                MODULE ": Value '%s' conforms to type of variable '%s'.\n",
                value,
                inst_name
            );
        }
        else {
            char const *errmsg = vartype_get_complete_error_message(THIS->var->type);
            if (*errmsg) {
                log_error(
                    MODULE ": Failed to set variable '%s' to '%s' as new value is not properly typed: %s\n",
                    inst_name,
                    value,
                    errmsg
                );
            }
            else {
                log_error(
                    MODULE ": Failed to set variable '%s' to '%s' as new value is not properly typed.\n",
                    inst_name,
                    value
                );
            }
            free(inst_name);
            return FALSE;
        }
    }

    if (value && slot->entry.value != value) {
        int old_bound = 0;
        if (THIS->var->num_var_n_clients > 0 && slot->entry.value != NULL) {
            char *p;
            errno = 0;
            old_bound = strtol(slot->entry.value, &p, 0);
            if (errno != 0 || *p != '\0') {
                old_bound = 0;
            }
        }

        log_info(
            LOG_VAR,
            MODULE ": Writing %s '%s' (priority %d) to variable '%s'.\n",
            check ? "checked" : "unchecked",
            value,
            priority,
            inst_name
        );
        free(slot->entry.value);
        slot->entry.value = strsave(value);

        if (THIS->var->num_var_n_clients > 0) {
            int new_bound = 0;
            char *p;
            errno = 0;
            new_bound = strtol(value, &p, 0);
            if (errno != 0 || *p != '\0') {
                new_bound = 0;
            }

            if (old_bound < new_bound) {
                struct var_t **v = THIS->var->var_n_clients;
                int i;

                for (i = 0; i < THIS->var->num_var_n_clients; ++i, ++v) {
                    struct var_instance_t *inst
                        = var_instance_create(*v, THIS->num_indices + 1, NULL);
                    memcpy(inst->indices, THIS->indices, THIS->num_indices * sizeof(unsigned));
                    /* note that indices are zero-based */
                    inst->indices[THIS->num_indices] = new_bound - 1;
                    var_slot_get_or_create(inst);
                    var_instance_destroy(inst);
                }
            }
        }
    }
    if (extra && slot->entry.extra != extra) {
        log_info(
            LOG_VAR,
            MODULE ": Updating extra data (priority %d) of variable '%s'.\n",
            priority,
            inst_name
        );
        if (slot->entry.extra) {
            var_slot_extra_destroy(slot->entry.extra);
        }
        slot->entry.extra = extra;
    }
    slot->entry.priority = priority;

    free(inst_name);
    return TRUE;
}

BOOL
var_instance_set_and_check_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority
)
{
    if (!value) {
        char *inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Cannot set variable '%s' to an undefined value.\n",
            inst_name
        );
        free(inst_name);
        return FALSE;
    }

    return var_instance_do_set_value(THIS, value, extra, mode, priority, TRUE, FALSE);
}

BOOL
var_instance_set_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority
)
{
    if (!value && !extra) {
        return TRUE;
    }

    return var_instance_do_set_value(THIS, value, extra, mode, priority, FALSE, FALSE);
}

BOOL
var_instance_clear(
    struct var_instance_t *THIS,
    enum var_access_mode_t mode,
    enum var_priority_t priority
)
{
    union varslot_t *slot;
    BOOL access_failed = FALSE;
    BOOL result;

    if (THIS->num_indices == THIS->var->level) {
        char *inst_name = var_instance_to_string(THIS);
        log_error(
            MODULE ": Cannot clear non-array variable '%s'.\n",
            inst_name
        );
        free(inst_name);
        return FALSE;
    }

    slot = var_instance_get_slot_for_writing(
        THIS, mode, priority, FALSE, &access_failed
    );
    if (!slot) {
        return access_failed && mode == VAR_ACCESS_TENTATIVE;
    }
    else {
        struct var_t *var_n;
        int index;
        struct var_instance_t *inst_n;

        var_slot_destroy(slot, THIS->num_indices, THIS->var->level);

        var_n = THIS->var->var_n;
        index = THIS->num_indices + 1;
        while (index < THIS->var->level) {
            var_n = var_n->var_n;
            ++index;
        }

        inst_n = var_instance_create(var_n, THIS->num_indices, THIS->indices);
        result = var_instance_do_set_value(inst_n, "0", NULL,
                VAR_ACCESS_TENTATIVE, VAR_PRIORITY_ARRAY_BOUND, TRUE, TRUE);
        my_assert_var(result);
        var_instance_destroy(inst_n);
        return TRUE;
    }
}
