/*****************************************************************************
 *  @file vartype/initfini.c
 *  Functions for initializing and finalizing the variable type system.
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
#include "regex.h"
#include "predef.h"

#define MODULE "vartype::initfini"

void
vartype_init_module(void)
{
    vartype_core_init();
    vartype_create_predefined_types();
}

void
vartype_fini_module(void)
{
    vartype_core_fini();
}

/**
 * Compiles a variable type by calling vartype_compile().
 *
 * @param entry
 *  The variable type.
 * @param data
 *  Points to a BOOL which is set to FALSE if compilation was unsuccessful.
 * @return
 *  Always TRUE.
 */
static BOOL
vartype_compile_action(void *entry, void *data)
{
    struct vartype_t *vartype = (struct vartype_t *) entry;
    if (!vartype_compile(vartype)) {
        *(BOOL *) data = FALSE;
    }
    return TRUE;
}

BOOL
vartype_finalize_type_system(void)
{
    DECLARE_ARRAY_ITER(it, entry, struct vartype_t);
    BOOL result = TRUE;

    hashmap_foreach(vartypes, &vartype_compile_action, &result);
    ARRAY_ITER_LOOP(it, anon_vartypes, entry) {
        if (!vartype_compile((struct vartype_t *) entry)) {
            result = FALSE;
        }
    }

    return result;
}
