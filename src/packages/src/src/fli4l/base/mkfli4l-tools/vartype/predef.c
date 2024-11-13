/*****************************************************************************
 *  @file vartype/predef.c
 *  Functions for processing predefined variable types.
 *
 *  Copyright (c) 2012-2016 The fli4l team
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
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "predef.h"
#include "regex.h"

#define MODULE "vartype::predef"

/**
 * Creates a predefined type. A predefined type may be overridden by a
 * definition with the same regular expression.
 *
 * @param name
 *  The name of the type.
 * @param expr
 *  The regular expression associated with this type.
 * @param error_msg
 *  The error message associated with this type.
 * @param comment
 *  A comment associated with this type.
 */
static void
vartype_create_predefined_type(
    char const *name,
    char const *expr,
    char const *error_msg,
    char const *comment
)
{
    struct vartype_t *const vartype = vartype_add(
        name,
        expr,
        error_msg,
        comment,
        location_create_internal(PACKAGE_FILE_TYPE_VARTYPE)
    );
    BOOL result;
    my_assert(vartype != NULL);
    vartype->predefined = TRUE;
    result = vartype_compile(vartype);
    my_assert_var(result);
}

void
vartype_create_predefined_types(void)
{
    vartype_create_predefined_type(
        VARTYPE_PREDEFINED_ANYTHING,
        ".*",
        "",
        "matches any value"
    );
    vartype_create_predefined_type(
        VARTYPE_PREDEFINED_UNSIGNED_INTEGER,
        "0|[1-9][0-9]*",
        "unsigned number expected",
        "unsigned integer number"
    );
    vartype_create_predefined_type(
        VARTYPE_PREDEFINED_BOOLEAN,
        "yes|no",
        "yes or no expected",
        "boolean value (yes or no)"
    );
}
