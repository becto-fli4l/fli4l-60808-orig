/*****************************************************************************
 *  @file file.c
 *  Functions for handling files.
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "libmkfli4l/str.h"
#include "libmkfli4l/file.h"

BOOL
get_file_name(char const *dir, char const *name, char const *ext, char **result)
{
    int len = strlen(dir) + 1 + strlen(name) + 1;
    if (ext) {
        len += strlen(ext);
    }

    *result = (char *) safe_malloc(len);
    strcpy(*result, dir);
    strcat(*result, "/");
    strcat(*result, name);
    if (ext) {
        strcat(*result, ext);
    }

    return access(*result, R_OK) == 0;
}
