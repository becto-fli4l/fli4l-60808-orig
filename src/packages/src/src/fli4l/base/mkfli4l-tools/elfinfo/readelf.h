/*----------------------------------------------------------------------------
 *  readelf.h - functions for the analysis of ELF files
 *
 *  Copyright (c) 2012-2016 The fli4l team
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

#ifndef MKFLI4L_READELF_H_
#define MKFLI4L_READELF_H_

#define READELF_ERROR_USAGE 1
#define READELF_ERROR_OUT_OF_MEMORY 2
#define READELF_ERROR_FILE_NOT_FOUND 3
#define READELF_ERROR_DATA_COULD_NOT_BE_READ 4
#define READELF_ERROR_FILE_HEADER_INVALID 5

/**
 * Describes the relevant information of an ELF file.
 */
struct readelf_descriptor {
    char *soname;
    char *rpath;
    int num_needed;
    char **needed;
};

/**
 * Reads DT_SONAME, DT_RPATH, and DT_NEEDED entries from the first section in
 * the DYNAMIC segment and returns them in a readelf_descriptor structure.
 *
 * Returns 0 on success and a READELF_ERROR_* code otherwise. Only if zero
 * is returned will *result point to a valid structure.
 */
int readelf_process_file (char const *file_name, struct readelf_descriptor **result);

/**
 * Frees the memory associated with a readelf_descriptor structure.
 */
void readelf_free_descriptor(struct readelf_descriptor *desc);

#endif
