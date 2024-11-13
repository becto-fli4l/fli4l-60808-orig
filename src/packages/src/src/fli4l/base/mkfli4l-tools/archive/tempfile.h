/*----------------------------------------------------------------------------
 *  tempfile.h - functions for creating and destroying temporary files
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

#ifndef MKFLI4L_TEMPFILE_H_
#define MKFLI4L_TEMPFILE_H_

/**
 * Describes a temporary file.
 */
struct tempfile_t {
    /**
     * Name of temporary file. NULL if unknown.
     */
    char *name;
    /**
     * Associated FILE object. Always non-NULL.
     */
    FILE *stream;
};

/**
 * Creates a temporary file.
 * @return
 *  An object describing the temporary file. The file is opened in binary mode
 *  for reading and writing.
 */
struct tempfile_t *create_temporary_file(void);

/**
 * Destroys a temporary file previously created by create_temporary_file().
 * @param desc
 *  An object describing the temporary file to destroy.
 */
void destroy_temporary_file(struct tempfile_t *desc);

#endif
