/*****************************************************************************
 *  @file var/slot_iter.c
 *  Functions for processing variable slot iterators.
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
#include "core.h"
#include "slot.h"
#include "slot_read.h"
#include "iter.h"

#define MODULE "var::iter"

static struct varslot_iterator_t *
var_slot_iterator_create(union varslot_t *THIS, struct var_t *var, int level)
{
    struct varslot_iterator_t *const iter = (struct varslot_iterator_t *)
        safe_malloc(sizeof(struct varslot_iterator_t));
    iter->parent = NULL;
    iter->var = var;
    iter->slot = THIS;
    iter->level = level;
    iter->i = -1;
    return iter;
}

static void
var_slot_iterator_destroy(struct varslot_iterator_t *THIS)
{
    free(THIS);
}

static struct varslot_iterator_t *
var_slot_iterator_next(struct varslot_iterator_t *THIS)
{
    if (THIS->i < 0) {
        THIS->i = 0;
        return THIS;
    }
    else if (THIS->level == THIS->var->level
                || (unsigned) THIS->i == THIS->slot->array.num_slots) {
        struct varslot_iterator_t *const parent = THIS->parent;
        if (parent) {
            var_slot_iterator_destroy(THIS);
            ++parent->i;
            return parent;
        }
        else {
            THIS->i = -1;
            return THIS;
        }
    }
    else {
        struct varslot_iterator_t *const next = (struct varslot_iterator_t *)
            safe_malloc(sizeof(struct varslot_iterator_t));
        next->parent = THIS;
        next->slot = &THIS->slot->array.slots[THIS->i];
        next->var = THIS->var;
        next->level = THIS->level + 1;
        next->i = 0;
        return next;
    }
}

struct var_value_iterator_t *
var_value_iterator_create(struct var_instance_t *THIS, BOOL all)
{
    union varslot_t *const slot = var_instance_get_slot_for_reading(THIS, TRUE);
    struct var_value_iterator_t *const iter = (struct var_value_iterator_t *)
        safe_malloc(sizeof(struct var_value_iterator_t));

    iter->inst = var_instance_create(THIS->var, THIS->var->level, NULL);
    iter->last = NULL;
    iter->all = all;

    if (slot) {
        int i;
        iter->iter = var_slot_iterator_create(slot, THIS->var, THIS->num_indices);
        iter->level = THIS->num_indices;

        for (i = 0; i < THIS->num_indices; ++i) {
            iter->inst->indices[i] = THIS->indices[i];
        }
    }
    else {
        iter->iter = NULL;
    }
    return iter;
}

void
var_value_iterator_destroy(struct var_value_iterator_t *THIS)
{
    struct varslot_iterator_t *iter = THIS->iter;
    while (iter) {
        struct varslot_iterator_t *const parent = iter->parent;
        var_slot_iterator_destroy(iter);
        iter = parent;
    }

    if (THIS->last != NULL) {
        var_value_destroy(THIS->last);
    }

    var_instance_destroy(THIS->inst);
    free(THIS);
}

struct var_t *
var_value_iterator_get_var(struct var_value_iterator_t *THIS)
{
    return THIS->inst->var;
}

unsigned *
var_value_iterator_get_var_indices(struct var_value_iterator_t *THIS)
{
    if (THIS->iter && THIS->iter->i >= 0) {
        return THIS->inst->indices;
    }
    else {
        return NULL;
    }
}

struct var_instance_t *
var_value_iterator_get_var_instance(struct var_value_iterator_t *THIS)
{
    if (THIS->iter && THIS->iter->i >= 0) {
        return THIS->inst;
    }
    else {
        return NULL;
    }
}

struct var_value_t *
var_value_iterator_get(struct var_value_iterator_t *THIS)
{
    return THIS->last;
}

BOOL
var_value_iterator_next(struct var_value_iterator_t *THIS)
{
    if (THIS->iter) {
        int const maxlevel = THIS->inst->var->level;

        if (THIS->last != NULL) {
            var_value_destroy(THIS->last);
            THIS->last = NULL;
        }

        while (!THIS->last || (!THIS->all && !THIS->last->value)) {
            if (THIS->last != NULL) {
                var_value_destroy(THIS->last);
                THIS->last = NULL;
            }

            do {
                THIS->iter = var_slot_iterator_next(THIS->iter);
            } while (THIS->iter->i >= 0 && THIS->iter->level != maxlevel);

            if (THIS->iter->i >= 0) {
                struct varslot_iterator_t *iter = THIS->iter->parent;
                while (iter) {
                    THIS->inst->indices[iter->level] = iter->i;
                    iter = iter->parent;
                }

                THIS->last
                    = var_instance_read_value(THIS->inst, THIS->iter->slot, FALSE);
            }
            else {
                return FALSE;
            }
        }

        return TRUE;
    }
    else {
        return FALSE;
    }
}
