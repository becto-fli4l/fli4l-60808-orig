/*****************************************************************************
 *  @file var/props.c
 *  Functions for processing variable properties.
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
#include <expression.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

#define MODULE "var::props"

struct var_properties_t *
var_properties_create(void)
{
    struct var_properties_t *result = (struct var_properties_t *)
        safe_malloc(sizeof(struct var_properties_t));

    result->comment = strsave("");
    result->optional = FALSE;
    result->weak = FALSE;
    result->condition
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    result->def_value = NULL;
    result->dis_value = NULL;
    result->extra = NULL;
    return result;
}

struct var_properties_t *
var_properties_clone(struct var_properties_t const *THIS)
{
    struct var_properties_t *result = (struct var_properties_t *)
        safe_malloc(sizeof(struct var_properties_t));

    result->comment = strsave(THIS->comment);
    result->optional = THIS->optional;
    result->weak = THIS->weak;
    result->condition = expr_clone(THIS->condition);
    result->def_value = THIS->def_value ? expr_clone(THIS->def_value) : NULL;
    result->dis_value = THIS->dis_value ? strsave(THIS->dis_value) : NULL;
    result->extra = THIS->extra ? var_extra_clone(THIS->extra) : NULL;
    return result;
}

void
var_properties_set_comment(
    struct var_properties_t *THIS,
    char const *comment
)
{
    my_assert(comment != NULL);
    free(THIS->comment);
    THIS->comment = strsave(comment);
}

void
var_properties_set_default_value(
    struct var_properties_t *THIS,
    struct expr_t *def_value
)
{
    if (THIS->def_value) {
        expr_destroy(THIS->def_value);
    }
    THIS->def_value = def_value;
}

void
var_properties_set_disabled_value(
    struct var_properties_t *THIS,
    char const *dis_value
)
{
    free(THIS->dis_value);
    THIS->dis_value = strsave(dis_value);
}

void
var_properties_set_condition(
    struct var_properties_t *THIS,
    struct expr_t *condition
)
{
    my_assert(condition != NULL);
    expr_destroy(THIS->condition);
    THIS->condition = condition;
}

void
var_properties_set_extra(
    struct var_properties_t *THIS,
    struct var_extra_t *extra
)
{
    if (THIS->extra) {
        var_extra_destroy(THIS->extra);
    }
    THIS->extra = extra;
}

BOOL
var_properties_check(struct var_properties_t const *THIS, char const *name)
{
    /* currently, there is nothing to check */
    return TRUE;

    UNUSED(THIS);
    UNUSED(name);
}

void
var_properties_destroy(struct var_properties_t *THIS)
{
    if (THIS->extra) {
        var_extra_destroy(THIS->extra);
    }
    expr_destroy(THIS->condition);
    free(THIS->dis_value);
    if (THIS->def_value) {
        expr_destroy(THIS->def_value);
    }
    free(THIS->comment);
    free(THIS);
}
