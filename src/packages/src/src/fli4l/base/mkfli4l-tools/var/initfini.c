/*****************************************************************************
 *  @file var/initfini.c
 *  Functions for initializing and finalizing the variable system.
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
#include <libmkfli4l/log.h>
#include <package.h>
#include <vartype.h>
#include "core.h"
#include "scope.h"

#define MODULE "var::initfini"

struct vartype_t *var_varname_type = NULL;

void
var_init_module(void)
{
    var_scope_init();

    var_varname_type = vartype_add_temporary(
        VAR_NAME_REGEX,
        location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK),
        FALSE
    );
    my_assert(var_varname_type);
}

void
var_fini_module(void)
{
    vartype_remove(var_varname_type);
    var_varname_type = NULL;

    var_scope_fini();
}
