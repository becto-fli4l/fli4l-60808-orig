/*****************************************************************************
 *  @file package/file.h
 *  Functions for processing package files (private interface).
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

#ifndef MKFLI4L_PACKAGE_FILE_H_
#define MKFLI4L_PACKAGE_FILE_H_

#include <libmkfli4l/defs.h>
#include <package.h>

/**
 * Represents a package file.
 */
struct package_file_t {
    /**
     * The package this file belongs to.
     */
    struct package_t *package;
    /**
     * The type of the file.
     */
    unsigned type;
    /**
     * The name of the file.
     */
    char *name;
};

/**
 * Creates a package file.
 *
 * @param package
 *  The package this file belongs to.
 * @param type
 *  The type of this file.
 * @param name
 *  The name of this file.
 * @return
 *  A pointer to the new package_file_t object.
 */
struct package_file_t *
package_file_create(
    struct package_t *package,
    unsigned type,
    char const *name
);

/**
 * Destroys a package file.
 *
 * @param THIS
 *  The package file.
 */
void
package_file_destroy(struct package_file_t *THIS);

#endif
