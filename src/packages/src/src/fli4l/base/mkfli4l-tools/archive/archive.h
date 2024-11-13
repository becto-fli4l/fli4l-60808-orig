/*----------------------------------------------------------------------------
 *  archive.h - functions for creating archives
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

#if !defined (MKFLI4L_ARCHIVE_H_)
#define MKFLI4L_ARCHIVE_H_

#include <stdio.h>
#include <libmkfli4l/defs.h>
#include <compression/arch_comp.h>
#include "arch_type.h"

struct entry_t;
struct archive_t;

/**
 * Creates an archive.
 * @param type_backend
 *  The archive type backend.
 * @param comp_backend
 *  The archive compression backend.
 * @param name
 *  The name of the archive file without a path.
 * @param destdir
 *  The name of the destination directory where the archive is to be created.
 * @param tmpdir
 *  The name of the directory where temporary files may be created.
 * @return
 *  A pointer to an archive_t object or NULL if an error occurred.
 */
struct archive_t *create_archive(struct arch_type_backend_t *type_backend, struct arch_comp_backend_t *comp_backend, char const *name, char const *destdir, char const *tmpdir);

/**
 * Adds an entry to some archive.
 * @param archive
 *  The archive to be extended.
 * @param entry
 *  The entry to add.
 * @return
 *  OK on success, ERR on failure.
 */
int add_to_archive(struct archive_t *archive, struct entry_t *entry);

/**
 * Closes an archive.
 * @param archive
 *  The archive to close.
 * @return
 *  OK on success, ERR on failure.
 */
int close_archive(struct archive_t *archive);

/**
 * Determines whether an entry is to be added to an archive.
 * @param entry
 *  The entry.
 * @param data
 *  Additional data for the filter function.
 * @return
 *  TRUE if entry is to be added to the archive, FALSE otherwise.
 */
typedef BOOL (*entry_filter_t)(struct entry_t *entry, void *data);

/**
 * Creates a complete archive.
 * @param name
 *  The name of the archive file without a path.
 * @param destdir
 *  The name of the destination directory where the archive is to be created.
 * @param tmpdir
 *  The name of the directory where temporary files may be created.
 * @param type_backend
 *  The archive type backend.
 * @param comp_backend
 *  The archive compression backend.
 * @param filter
 *  A filter that determines whether an entry will be included in the archive.
 * @param filter_data
 *  Additional data for the filter function.
 * @return
 *  OK on success, ERR on failure.
 */
int create_complete_archive(char const *name, char const *destdir, char const *tmpdir, struct arch_type_backend_t *type_backend, struct arch_comp_backend_t *comp_backend, entry_filter_t filter, void *filter_data);

#endif
