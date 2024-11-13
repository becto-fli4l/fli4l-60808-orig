/*****************************************************************************
 *  @file var/inst.c
 *  Functions for processing variable instantiations.
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
#include <ctype.h>
#include <regex.h>
#include "inst.h"
#include "core.h"
#include "scope.h"

#define MODULE "var::inst"

/**
 * Array containing all digits.
 */
static char const digits[] = "0123456789";

struct var_instance_t *
var_instance_create(
    struct var_t *THIS,
    int num_indices,
    unsigned const *indices
)
{
    struct var_instance_t *instance = (struct var_instance_t *)
        safe_malloc(sizeof(struct var_instance_t));
    instance->var = THIS;
    instance->num_indices = num_indices;
    instance->indices = num_indices > 0
        ? (unsigned *) safe_malloc(num_indices * sizeof(unsigned)) : NULL;
    if (indices) {
        memcpy(instance->indices, indices, num_indices * sizeof(unsigned));
    }
    else {
        memset(instance->indices, 0, num_indices * sizeof(unsigned));
    }
    var_register_instance(THIS, instance);
    return instance;
}

void
var_instance_invalidate(struct var_instance_t *THIS)
{
    THIS->var = NULL;
}

BOOL
var_instance_is_valid(struct var_instance_t const *THIS)
{
    return THIS->var != NULL;
}

void
var_instance_destroy(struct var_instance_t *THIS)
{
    if (var_instance_is_valid(THIS)) {
        var_unregister_instance(THIS->var, THIS);
    }
    free(THIS->indices);
    free(THIS);
}

char *
var_instance_to_string(struct var_instance_t const *THIS)
{
    if (var_instance_is_valid(THIS)) {
        char *result = strsave("");
        char const *p = var_get_name(THIS->var);
        char const *q;
        int level = 0;
        unsigned const *indices = THIS->indices;

        while ((q = strchr(p, VAR_PLACEHOLDER)) != NULL) {
            char *s;

            s = substrsave(p, q);
            result = strcat_save(result, s);
            free(s);

            if (level < THIS->num_indices) {
                s = safe_sprintf("%u", *indices + 1);
                result = strcat_save(result, s);
                free(s);
                ++indices;
            }
            else {
                char placeholder[2];
                placeholder[0] = VAR_PLACEHOLDER;
                placeholder[1] = '\0';
                result = strcat_save(result, placeholder);
            }

            ++level;
            p = q + 1;
        }

        return strcat_save(result, p);
    }
    else {
        return NULL;
    }
}

BOOL
var_instance_is_enabled(struct var_instance_t *THIS)
{
    if (var_instance_is_valid(THIS)) {
        BOOL enabled;
        char *error_msg;
        struct expr_t *cond = var_get_condition(THIS->var);
        struct expr_literal_t *result;
        struct expr_eval_context_t *context = expr_eval_context_create();
        context->var_inst = THIS;

        result = expr_evaluate(cond, context);
        if (!expr_get_boolean_value(result, &enabled, &error_msg)) {
            free(error_msg);
            enabled = FALSE;
        }
        expr_destroy(expr_literal_to_expr(result));

        expr_eval_context_destroy(context);
        return enabled;
    }
    else {
        return FALSE;
    }
}

struct var_instance_t *
var_instance_create_from_string(struct var_scope_t *scope, char const *name)
{
    struct var_t *var = var_scope_try_get(scope, name, TRUE);
    if (var) {
        return var_instance_create(var, 0, NULL);
    }
    else {
        struct var_instance_t *instance;
        int num_indices = 0;
        unsigned *indices = NULL;

        char *var_name = (char *) safe_malloc(strlen(name) + 1);
        char *end = var_name;
        char const *p = name;
        char const *q = name;
        while ((q = strchr(q, VAR_STRUCTURE_SEPARATOR)) != NULL) {
            if (isdigit(q[1])) {
                ++q;
                indices = (unsigned *)
                    safe_realloc(indices, (num_indices + 1) * sizeof(unsigned));
                indices[num_indices++] = atoi(q) - 1;

                strcpy(end, p);
                end += q - p;
                *end++ = VAR_PLACEHOLDER;

                p = q = q + strspn(q, digits);
            }
            else {
                ++q;
            }
        }
        strcpy(end, p);

        var = var_scope_try_get(scope, var_name, TRUE);
        free(var_name);
        if (var) {
            instance = var_instance_create(var, num_indices, indices);
            free(indices);
            return instance;
        }
        else {
            free(indices);
            return NULL;
        }
    }
}
