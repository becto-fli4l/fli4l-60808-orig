/*****************************************************************************
 *  @file package/core.h
 *  Functions for processing packages (private interface).
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

#ifndef MKFLI4L_PACKAGE_CORE_H_
#define MKFLI4L_PACKAGE_CORE_H_

#include <libmkfli4l/defs.h>
#include <package.h>
#include "file.h"

/**
 * Number of package file types.
 */
#define PACKAGE_FILE_TYPE_NUM (PACKAGE_FILE_TYPE_IMAGE + 1)

/**
 * Represents a package.
 */
struct package_t {
    /**
     * The package name.
     */
    char *name;
    /**
     * Miscellaneous package files. Currently, every package can only have at
     * most one file per type.
     */
    struct package_file_t *files[PACKAGE_FILE_TYPE_NUM];
};

/**
 * Hash map of package_map indexed by name.
 */
extern struct hashmap_t *package_map;

/**
 * Initializes the core module.
 */
void
package_core_init(void);

/**
 * Finalizes the core module.
 */
void
package_core_fini(void);

#endif
