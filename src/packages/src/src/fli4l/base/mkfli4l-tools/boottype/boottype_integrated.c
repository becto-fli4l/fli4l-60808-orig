/*----------------------------------------------------------------------------
 *  boottype_integrated.c - functions for handling integrated boot types
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

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/options.h>
#include <archive/archive.h>
#include <archive/arch_type_cpio.h>
#include <ziplist/opt.h>
#include "boottype_integrated.h"

#define RC_CFG_OPTS "name=boot/rc.cfg mode=600"

static int
add_integrated_boot_type_entries(struct boot_type_backend_t *backend)
{
    (void) backend;
    return add_zip_list_entry(rc_file, ZIP_SRC_OTHER, ZIP_TGT_ROOTFS,
            ENTRY_FILE, parse_entry_options(RC_CFG_OPTS), "") != NULL
                ? OK : ERR;
}

static BOOL
filter_all(struct entry_t *entry, void *data)
{
    (void) entry;
    (void) data;
    return TRUE;
}

static int
create_integrated_boot_type_archives(struct boot_type_backend_t *backend, struct arch_comp_backend_t *comp_rootfs_backend, struct arch_comp_backend_t *comp_opt_backend)
{
    int rc;

    if ((rc = create_complete_archive(rootfs_img_file,
                                      build_dir,
                                      scratch_dir,
                                      create_cpio_backend(),
                                      comp_rootfs_backend,
                                      filter_all,
                                      NULL)) != OK) {
        return rc;
    }

    (void) backend;
    (void) comp_opt_backend;
    return OK;
}

static void
destroy_integrated_boot_type_backend(struct boot_type_backend_t *backend)
{
    (void) backend;
}

struct boot_type_backend_t *
create_integrated_boot_type_backend(void)
{
    struct boot_type_backend_t *backend = (struct boot_type_backend_t *)
            safe_malloc(sizeof(struct boot_type_backend_t));

    backend->add_entries = &add_integrated_boot_type_entries;
    backend->create_archives = &create_integrated_boot_type_archives;
    backend->destroy_backend = &destroy_integrated_boot_type_backend;
    backend->extra_data = NULL;

    return backend;
}
