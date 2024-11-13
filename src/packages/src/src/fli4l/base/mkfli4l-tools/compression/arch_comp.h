/*----------------------------------------------------------------------------
 *  arch_comp.h - functions for archive compression backends
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

#if !defined (MKFLI4L_ARCH_COMP_H_)
#define MKFLI4L_ARCH_COMP_H_

#include <stddef.h>

/**
 * Describes an archive compression backend.
 */
struct arch_comp_backend_t {
    /**
     * Method creating a copmpressed archive.
     * @param backend
     *  The archive compression backend.
     * @param out
     *  The name of the file to receive the compressed archive contents.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*create_archive)(struct arch_comp_backend_t *backend, char const *out);
    /**
     * Reads, compresses, and writes data.
     * @param backend
     *  The archive compression backend.
     * @param buf
     *  The data to compress.
     * @param buflen
     *  The number of bytes to compress.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*compress_data)(struct arch_comp_backend_t *backend, void const *buf, size_t buflen);
    /**
     * Method closing the compressed archive.
     * @param backend
     *  The archive compression backend.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*close_archive)(struct arch_comp_backend_t *backend);
    /**
     * Method destroying the backend.
     * @param backend
     *  The archive type backend.
     */
    void (*destroy_backend)(struct arch_comp_backend_t *backend);
    /**
     * Extra data depending on the archive compression backend.
     */
    void *extra_data;
};

#endif
