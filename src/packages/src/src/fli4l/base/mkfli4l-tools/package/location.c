/*****************************************************************************
 *  @file package/location.c
 *  Functions for managing package file locations.
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
 *****************************************************************************
 */

#include <libmkfli4l/defs.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/str.h>
#include <package.h>
#include <stdlib.h>

#define MODULE "package::location"

/**
 * Describes a location.
 */
struct location_t {
    /**
     * The file.
     */
    struct package_file_t *file;
    /**
     * The line in the file.
     */
    int line;
    /**
     * The column in the file.
     */
    int column;
};

struct location_t *
location_create(struct package_file_t *file, int line, int column)
{
    struct location_t *location;

    my_assert(file != NULL);

    location =
        (struct location_t *) safe_malloc(sizeof(struct location_t));
    location->file = file;
    location->line = line;
    location->column = column;
    return location;
}

struct location_t *
location_create_internal(unsigned type)
{
    return location_create(package_get_file(package_get_internal(), type), 0, 0);
}

BOOL
location_equals(struct location_t const *THIS, struct location_t const *other)
{
    return
        THIS->file == other->file &&
        THIS->line == other->line &&
        THIS->column == other->column;
}

char *
location_toString(struct location_t const *location)
{
    char *file_str = package_file_to_string(location->file);
    char *result = safe_sprintf(
        "[%s %d:%d]", file_str, location->line, location->column
    );
    free(file_str);
    return result;
}

struct location_t *
location_clone(struct location_t const *location)
{
    return location_create(location->file, location->line, location->column);
}

struct package_file_t *
location_get_package_file(struct location_t const *location)
{
    return location->file;
}

void
location_destroy(struct location_t *location)
{
    free(location);
}
