/*****************************************************************************
 *  @file kernmod.h
 *  Functions for resolving kernel module dependencies.
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
 *  Creation:    2000-05-03  fm
 *  Last Update: $Id$
 *****************************************************************************
 */

#ifndef MKFLI4L_KERNMOD_H_
#define MKFLI4L_KERNMOD_H_

/**
 * Initializes the kernmod module.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int kernmod_init(char const *kernel_version);

/**
 * Tries to map a module name (which may be an alias) to fully qualified
 * module paths.
 *
 * @param name
 *  The module name to look up.
 * @return
 *  An array of fully qualified module path or NULL if the module or alias
 *  could not be found.
 */
extern char **map_module_name(char const *name);

/**
 * Resolves module dependencies by adding modules that are directly or
 * indirectly referenced by other modules already included in the archive.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int resolve_dependencies (char const *kernel_version);

#endif
