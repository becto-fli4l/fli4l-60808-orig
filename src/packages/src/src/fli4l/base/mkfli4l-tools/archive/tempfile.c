/*----------------------------------------------------------------------------
 *  tempfile.c - functions for creating and destroying temporary files
 *
 *  Copyright (c) 2012-2016 - fli4l-Team <team@fli4l.de>
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
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/options.h>
#include "tempfile.h"

struct tempfile_t *
create_temporary_file(void)
{
    struct tempfile_t *desc = (struct tempfile_t *) safe_malloc(sizeof(struct tempfile_t));
#if defined(PLATFORM_mingw32)
    desc->name = tempnam(scratch_dir, "mkfli4l-");
    desc->stream = desc->name ? fopen(desc->name, "w+b") : NULL;
#else
    desc->name = NULL;
    desc->stream = tmpfile();
#endif
    if (desc->stream != NULL) {
        return desc;
    }
    else {
        free(desc->name);
        free(desc);
        return NULL;
    }
}

void
destroy_temporary_file(struct tempfile_t *desc)
{
    fclose(desc->stream);
#if defined(PLATFORM_mingw32)
    remove(desc->name);
#endif
    free(desc->name);
    free(desc);
}
