/*----------------------------------------------------------------------------
 *  boottype.h - functions for handling different boot types
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

#ifndef MKFLI4L_BOOTTYPE_H_
#define MKFLI4L_BOOTTYPE_H_

#include <compression/arch_comp.h>

/**
 * Describes a boot type backend.
 */
struct boot_type_backend_t {
    /**
     * Adds entries specific to this boot type.
     * @param backend
     *  The boot type backend.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*add_entries)(struct boot_type_backend_t *backend);
    /**
     * Creates archives as needed by this boot type.
     * @param backend
     *  The boot type backend.
     * @param comp_rootfs_backend
     *  The ROOTFS compression backend.
     * @param comp_opt_backend
     *  The OPT compression backend.
     * @return
     *  OK on success, ERR on failure.
     */
    int (*create_archives)(struct boot_type_backend_t *backend, struct arch_comp_backend_t *comp_rootfs_backend, struct arch_comp_backend_t *comp_opt_backend);
    /**
     * Method destroying the backend.
     * @param backend
     *  The boot type backend.
     */
    void (*destroy_backend)(struct boot_type_backend_t *backend);
    /**
     * Extra data depending on the boot type backend.
     */
    void *extra_data;
};

/**
 * Initializes the boot type backend.
 */
extern void boot_type_init(void);

/**
 * Determines the compression used for the ROOTFS.
 * @param kernel_version
 *  The kernel version.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int determine_rootfs_compression(char const *kernel_version);

/**
 * Performs actions depending on the selected boot type.
 * @param boottype
 *  The name of the boot type.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int add_boot_type_specific_entries(char const *boottype);

/**
 * Creates the archives as needed by the selected boot type.
 * @param boottype
 *  The name of the boot type.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int create_archives(char const *boottype);

#endif
