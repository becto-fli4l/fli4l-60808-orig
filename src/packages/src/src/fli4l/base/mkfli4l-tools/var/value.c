/*****************************************************************************
 *  @file var/value.c
 *  Functions for processing values.
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
#include <errno.h>
#include "core.h"

#define MODULE "var::value"

struct var_value_t *
var_value_create(
    char const *value,
    enum var_value_kind_t kind,
    struct var_slot_extra_t *extra,
    enum var_priority_t priority
)
{
    struct var_value_t *const result
        = (struct var_value_t *) safe_malloc(sizeof(struct var_value_t));
    result->value = value;
    result->kind = kind;
    result->extra = extra;
    result->priority = priority;
    return result;
}

BOOL
var_value_convert_to_integer(struct var_value_t const *THIS, int *result)
{
    char *p;
    errno = 0;
    *result = strtol(THIS->value, &p, 0);
    return errno == 0 && *p == '\0';
}

BOOL
var_value_convert_to_boolean(struct var_value_t const *THIS, BOOL *result)
{
    if (strcmp(THIS->value, EXPR_LITERAL_STRING_TRUE) == 0) {
        *result = TRUE;
        return TRUE;
    }
    else if (strcmp(THIS->value, EXPR_LITERAL_STRING_FALSE) == 0) {
        *result = FALSE;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void
var_value_destroy(struct var_value_t *THIS)
{
    free(THIS);
}
