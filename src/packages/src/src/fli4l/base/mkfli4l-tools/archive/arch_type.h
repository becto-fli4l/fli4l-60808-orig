/*----------------------------------------------------------------------------
 *  arch_type.h - functions for archive type backends
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

#if !defined (MKFLI4L_ARCH_TYPE_H_)
#define MKFLI4L_ARCH_TYPE_H_

#include <stdio.h>

struct entry_t;

/**
 * Describes an archive type backend.
 */
struct arch_type_backend_t {
    /**
     * Method creating an archive.
     * @param backend
     *  The archive type backend.
     * @param out
     *  The name of the file to receive the archive contents.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*create_archive)(struct arch_type_backend_t *backend, char const *out);
    /**
     * Method adding a file entry (with contents) to the archive.
     * @param backend
     *  The archive type backend.
     * @param entry
     *  The entry to be added to the archive.
     * @param in
     *  The entry's contents to be added to the archive.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*add_file_to_archive)(struct arch_type_backend_t *backend, struct entry_t *entry, FILE *in);
    /**
     * Method adding a non-file entry (without contents) to the archive.
     * @param backend
     *  The archive type backend.
     * @param entry
     *  The entry to be added to the archive.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*add_non_file_to_archive)(struct arch_type_backend_t *backend, struct entry_t *entry);
    /**
     * Method closing an archive.
     * @param backend
     *  The archive type backend.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*close_archive)(struct arch_type_backend_t *backend);
    /**
     * Method destroying the backend.
     * @param backend
     *  The archive type backend.
     */
    void (*destroy_backend)(struct arch_type_backend_t *backend);
    /**
     * Extra data depending on the archive type backend.
     */
    void *extra_data;
};

#endif
