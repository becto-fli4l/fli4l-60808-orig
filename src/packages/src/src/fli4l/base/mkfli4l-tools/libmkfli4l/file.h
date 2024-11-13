/*****************************************************************************
 *  @file file.h
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

#ifndef LIBMKFLI4L_FILE_H_
#define LIBMKFLI4L_FILE_H_

#include <stdio.h>
#include "libmkfli4l/defs.h"
#include "libmkfli4l/array.h"

/**
 * Combines directory, file name, and file extension and checks whether this
 * file exists.
 *
 * @param dir
 *  The directory to be prepended to the file name.
 * @param name
 *  The name of the file to be looked up without extension.
 * @param ext
 *  An optional extension of the file to be looked up.
 * @param result
 *  Receives the path to the resulting file.
 * @return
 *  TRUE if the resulting file exists and can be read, FALSE otherwise.
 */
BOOL get_file_name(char const *dir, char const *name, char const *ext, char **result);

#endif
