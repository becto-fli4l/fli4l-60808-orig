/*****************************************************************************
 *  @file package/file.c
 *  Functions for processing package files.
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
#include <stdlib.h>
#include <string.h>
#include "file.h"

#define MODULE "package::file"

struct package_file_t *
package_file_create(
    struct package_t *package,
    unsigned type,
    char const *name
)
{
    struct package_file_t *const file
        = (struct package_file_t *) safe_malloc(sizeof(struct package_file_t));
    file->package = package;
    file->type = type;
    file->name = strsave(name);
    return file;
}

void
package_file_destroy(struct package_file_t *THIS)
{
    free(THIS->name);
    free(THIS);
}

struct package_t *
package_file_get_package(struct package_file_t *THIS)
{
    return THIS->package;
}

unsigned
package_file_get_type(struct package_file_t *THIS)
{
    return THIS->type;
}

char const *
package_file_get_name(struct package_file_t *THIS)
{
    return THIS->name;
}

char *
package_file_to_string(struct package_file_t *THIS)
{
    return safe_sprintf(
        "%s '%s' (package %s)",
        package_type_to_string(THIS->type),
        THIS->name,
        package_get_name(THIS->package)
    );
}
