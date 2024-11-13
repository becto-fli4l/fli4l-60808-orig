/*----------------------------------------------------------------------------
 *  boottype.c - functions for handling different boot types
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/array.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/options.h>
#include <compression/arch_comp_none.h>
#include <compression/arch_comp_gzip.h>
#include <compression/arch_comp_bz2.h>
#include <compression/arch_comp_lzma_xz.h>
#include "boottype.h"
#include "boottype_generic.h"
#include "boottype_integrated.h"
#include "boottype_attached.h"
#include "var.h"

#define BASE "base"
#define PREDEFINED "predefined variable"

#define VAR_COMP_TYPE_ROOTFS "COMP_TYPE_ROOTFS"
#define VAR_COMP_TYPE_ROOTFS_KV "COMP_TYPE_ROOTFS_V%s"
#define VAR_COMP_TYPE_OPT "COMP_TYPE_OPT"

#define BTMAP_ARRAY_SIZE       16
#define COMPMAP_ARRAY_SIZE     8

struct bt_map_entry_t {
    /**
     * The boot type name.
     */
    char const *name;
    /**
     * The boot type backend.
     */
    struct boot_type_backend_t *backend;
};

struct comp_map_entry_t {
    /**
     * The compression type.
     */
    char const *name;
    /**
     * The archive compression backend.
     */
    struct arch_comp_backend_t *backend;
};

static array_t *bt_map;
static array_t *comp_map;

static void
put_bt_map_entry(char const *name, struct boot_type_backend_t *backend)
{
    struct bt_map_entry_t *entry = (struct bt_map_entry_t *)
        safe_malloc(sizeof(struct bt_map_entry_t));
    entry->name = name;
    entry->backend = backend;
    append_new_elem(bt_map, entry);
}

static void
put_comp_map_entry(char const *name, struct arch_comp_backend_t *backend)
{
    struct comp_map_entry_t *entry = (struct comp_map_entry_t *)
        safe_malloc(sizeof(struct comp_map_entry_t));
    entry->name = name;
    entry->backend = backend;
    append_new_elem(comp_map, entry);
}

static void
init_bt_map(void)
{
    bt_map = init_array(BTMAP_ARRAY_SIZE);

    put_bt_map_entry("fd1440", create_generic_boot_type_backend());
    put_bt_map_entry("fd1680", create_generic_boot_type_backend());
    put_bt_map_entry("dual1440", create_generic_boot_type_backend());
    put_bt_map_entry("dual1680", create_generic_boot_type_backend());
    put_bt_map_entry("dualfd", create_generic_boot_type_backend());
    put_bt_map_entry("fd", create_generic_boot_type_backend());
    put_bt_map_entry("fdx2", create_generic_boot_type_backend());
    put_bt_map_entry("cd", create_generic_boot_type_backend());
    put_bt_map_entry("cdemul", create_generic_boot_type_backend());
    put_bt_map_entry("hd", create_generic_boot_type_backend());
    put_bt_map_entry("ls120", create_generic_boot_type_backend());

    put_bt_map_entry("integrated", create_integrated_boot_type_backend());
    put_bt_map_entry("netboot", create_integrated_boot_type_backend());
    put_bt_map_entry("pxeboot", create_integrated_boot_type_backend());

    put_bt_map_entry("attached", create_attached_boot_type_backend());
}

static void
init_comp_map(void)
{
    comp_map = init_array(COMPMAP_ARRAY_SIZE);

    put_comp_map_entry("uncompressed", create_comp_none_backend());
    put_comp_map_entry("gzip", create_comp_gzip_backend());
    put_comp_map_entry("bzip2", create_comp_bz2_backend());
    put_comp_map_entry("lzma", create_comp_lzma_backend());
    put_comp_map_entry("xz", create_comp_xz_backend());
}

static struct arch_comp_backend_t *
lookup_comp_backend(char const *name) {
    ARRAY_ITER(it, comp_map, entry, struct comp_map_entry_t) {
        if (strcmp(name, entry->name) == 0) {
            return entry->backend;
        }
    }

    return NULL;
}

static void
adjust_comp_type_rootfs_var_name(char *var_name)
{
    while (*var_name) {
        switch (*var_name) {
        case '.' :
            memmove(var_name, var_name + 1, strlen(var_name));
            break;
        case '-' :
            *var_name++ = '_';
            break;
        default :
            *var_name = toupper((int) *var_name);
            ++var_name;
        }
    }
}

void
boot_type_init(void)
{
    init_bt_map();
    init_comp_map();
}

int
determine_rootfs_compression(char const *kernel_version)
{
    char *comp_type_rootfs_var_name;
    char const *comp_type_rootfs;
    char *kv;
    char const *kv_postfix;
    int res = OK;

    struct var_t *var_comp_type_rootfs = var_get(VAR_COMP_TYPE_ROOTFS);
    if (!var_comp_type_rootfs) {
        log_error("Variable %s does not exist", VAR_COMP_TYPE_ROOTFS);
        return ERR;
    }

    kv = strsave(kernel_version);
    kv_postfix = strchr(kernel_version, '-');
    while (TRUE) {
        char *p;

        comp_type_rootfs_var_name = safe_sprintf(VAR_COMP_TYPE_ROOTFS_KV, kv);
        adjust_comp_type_rootfs_var_name(comp_type_rootfs_var_name);
        comp_type_rootfs = var_read(comp_type_rootfs_var_name);
        free(comp_type_rootfs_var_name);

        if (comp_type_rootfs) {
            struct var_instance_t *inst
                = var_instance_create(var_comp_type_rootfs, 0, NULL);
            res = var_instance_set_value(inst, comp_type_rootfs, NULL,
                    VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION) ? OK : ERR;
            var_instance_destroy(inst);
            break;
        }

        p = strrchr(kv, '.');
        if (p) {
            *p = '\0';
            if (kv_postfix) {
                strcat(kv, kv_postfix);
            }
        }
        else {
            break;
        }
    }

    free(kv);
    return res;
}

int
add_boot_type_specific_entries(char const *boottype)
{
    ARRAY_ITER(it, bt_map, entry, struct bt_map_entry_t) {
        if (strcmp(boottype, entry->name) == 0) {
            return entry->backend->add_entries(entry->backend);
        }
    }

    log_error("Unknown boot type %s\n", boottype);
    return ERR;
}

int
create_archives(char const *boottype)
{
    char const *comp_type_rootfs;
    char const *comp_type_opt;
    struct arch_comp_backend_t *rootfs_comp_backend;
    struct arch_comp_backend_t *opt_comp_backend;
    DECLARE_ARRAY_ITER(it, entry, struct bt_map_entry_t);

    comp_type_rootfs = var_read(VAR_COMP_TYPE_ROOTFS);
    if (comp_type_rootfs) {
        rootfs_comp_backend = lookup_comp_backend(comp_type_rootfs);
        if (!rootfs_comp_backend) {
            log_error("Unsupported ROOTFS compression '%s'\n", comp_type_rootfs);
            return ERR;
        }
        else {
            log_info(LOG_INFO | LOG_ARCHIVE, "Creating ROOTFS archive with compression '%s'\n", comp_type_rootfs);
        }
    }
    else {
        rootfs_comp_backend = create_comp_none_backend();
        log_info(LOG_INFO | LOG_ARCHIVE, "Creating uncompressed ROOTFS archive\n");
    }

    comp_type_opt = var_read(VAR_COMP_TYPE_OPT);
    if (comp_type_opt) {
        opt_comp_backend = lookup_comp_backend(comp_type_opt);
        if (!opt_comp_backend) {
            log_error("Unsupported OPT compression '%s'\n", comp_type_opt);
            return ERR;
        }
        else {
            log_info(LOG_INFO | LOG_ARCHIVE, "Creating OPT archive with compression '%s'\n", comp_type_opt);
        }
    }
    else {
        opt_comp_backend = create_comp_none_backend();
        log_info(LOG_INFO | LOG_ARCHIVE, "Creating uncompressed OPT archive\n");
    }

    ARRAY_ITER_LOOP(it, bt_map, entry) {
        if (strcmp(boottype, entry->name) == 0) {
            return entry->backend->create_archives(entry->backend, rootfs_comp_backend, opt_comp_backend);
        }
    }

    log_error("Unknown boot type %s\n", boottype);
    return ERR;
}
