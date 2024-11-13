/*****************************************************************************
 *  @file var/slot.c
 *  Functions for processing variable slots.
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
#include <expression/expr.h>
#include <expression/expr_literal.h>
#include <stdlib.h>
#include "core.h"
#include "slot.h"

#define MODULE "var::slot"

void
var_slot_init(union varslot_t *THIS, int level, int maxlevel)
{
    if (level == maxlevel) {
        THIS->entry.value = NULL;
        THIS->entry.def_value = NULL;
        THIS->entry.extra = NULL;
        THIS->entry.priority = VAR_PRIORITY_UNDEFINED;
        THIS->entry.read_active = FALSE;
    }
    else {
        THIS->array.num_slots = 0;
        THIS->array.slots = NULL;
    }
}

void
var_slot_destroy(union varslot_t *THIS, int level, int maxlevel)
{
    if (level == maxlevel) {
        free(THIS->entry.value);
        free(THIS->entry.def_value);
        if (THIS->entry.extra) {
            var_slot_extra_destroy(THIS->entry.extra);
        }
    }
    else {
        unsigned i;
        for (i = 0; i < THIS->array.num_slots; ++i) {
            var_slot_destroy(&THIS->array.slots[i], level + 1, maxlevel);
        }
        free(THIS->array.slots);
        THIS->array.slots = NULL;
        THIS->array.num_slots = 0;
    }
}

struct var_value_t *
var_slot_get_value(union varslot_t *THIS, struct var_instance_t *inst)
{
    struct var_value_t *result = var_value_create(
        NULL,
        VAR_VALUE_KIND_NONE,
        THIS->entry.extra,
        THIS->entry.priority
    );

    if (THIS->entry.value) {
        result->value = THIS->entry.value;
        result->kind = VAR_VALUE_KIND_NORMAL;
    }
    else {
        if (!THIS->entry.def_value) {
            struct expr_t *def_value = var_get_default_value(inst->var);
            if (def_value) {
                char *error_msg;
                struct expr_literal_t *eval_result;

                struct expr_eval_context_t *context = expr_eval_context_create();
                context->var_inst = inst;
                eval_result = expr_evaluate(def_value, context);

                if (!expr_get_string_value(eval_result,
                        &THIS->entry.def_value, &error_msg)) {
                    free(error_msg);
                }

                expr_destroy(expr_literal_to_expr(eval_result));
                expr_eval_context_destroy(context);
            }
        }

        if (THIS->entry.def_value) {
            result->value = THIS->entry.def_value;
            result->kind = VAR_VALUE_KIND_DEFAULT;
        }
    }

    return result;
}
