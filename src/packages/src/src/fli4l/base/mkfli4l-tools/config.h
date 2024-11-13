/*****************************************************************************
 *  @file config.h
 *  Functions for processing config/<package>.txt files.
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

#ifndef MKFLI4L_CONFIG_H_
#define MKFLI4L_CONFIG_H_

/**
 * Name of the 'base' package. The base package is processed even if a fixed
 * set of packages not containing 'base' is passed to mkfli4l. Also, its
 * extended checks are executed first.
 */
#define PKG_BASE "base"
/**
 * Name of the 'circuits' package. The extended checks of the 'circuits'
 * package are executed last.
 */
#define PKG_CIRCUITS "circuits"

/**
 * Processes configuration files and their dependencies.
 *
 * @param config_dir
 *  The directory containing configuration files.
 * @param check_dir
 *  The directory containing variable type files, variable definition
 *  files, and extended check scripts.
 * @param packages
 *  A NULL-terminated array of package names that are to be processed
 *  (only). If the array is empty, all matching package files (and an
 *  optionally existing superseding file) are processed.
 * @return
 *  TRUE if processing all matching package files in all directories was
 *  successful, FALSE otherwise.
 */
BOOL
read_config(char const *config_dir, char const *check_dir, char **packages);

#endif
