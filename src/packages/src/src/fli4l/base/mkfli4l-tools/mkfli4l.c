/*****************************************************************************
 *  @file mkfli4l.c
 *  Creates archives for fli4l.
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

#include <stdlib.h>
#include <signal.h>
#include <time.h>

#ifdef MALLOC_DEBUG
#include <mcheck.h>
#endif

#include <libmkfli4l/defs.h>
#include <libmkfli4l/var.h>
#include <libmkfli4l/regex.h>
#include <libmkfli4l/check.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/options.h>
#include <ziplist/opt.h>
#include <ziplist/optlibs.h>
#include <ziplist/kernmod.h>
#include <boottype/boottype.h>
#include <package.h>
#include <vartype.h>
#include <var.h>
#include "config.h"
#include "buildinfo.h"
#include "mk_syslinux.h"

#define VAR_BOOT_TYPE "BOOT_TYPE"

/**
 * Default signal handler.
 */
static void
catch(int sig)
{
    abort();
    (void) sig;
}

/**
 * The application's entry point.
 * @param argc
 *  The number of arguments.
 * @param argv
 *  A NULL-terminated array of arguments.
 * @return
 *  Zero on success, a non-zero value otherwise.
 */
int
main(int argc, char ** argv)
{
    int   res = OK;
    char const *kernel_version = NULL;
    BOOL  check_only = FALSE;
    BOOL  scanparse = TRUE;
#ifdef MALLOC_DEBUG
    mtrace();
#endif

    signal(SIGTERM, catch);
    signal(SIGINT, catch);

    srand((unsigned int) time(NULL));
    get_options(argc, argv);
    open_logfile(logfile);

    package_init_module();
    vartype_init_module();
    var_init_module();

    var_init();
    check_init();
    regexp_init();
    zip_list_init();
    boot_type_init();

    log_info(LOG_INFO, "Processing configuration...\n");
    inc_log_indent_level();

    check_only = *opt_packages != NULL;
    if (!read_config(config_dir, check_dir, opt_packages)) {
        fatal_exit("Error while reading configuration, aborting!\n");
    }

    log_info(LOG_INFO, "Processing variable type definitions...\n");
    if (!regexp_process_definitions()) {
        log_error("Error while processing variable type definitions.\n");
        scanparse = FALSE;
    }

    log_info(LOG_INFO, "Processing variable definitions...\n");
    if (!check_process_definitions()) {
        log_error("Error while processing variable definitions.\n");
        scanparse = FALSE;
    }

    log_info(LOG_INFO, "Processing variable template definitions...\n");
    if (!check_process_template_definitions()) {
        log_error("Error while processing variable template definitions.\n");
        scanparse = FALSE;
    }

    log_info(LOG_INFO, "Processing variable assignments...\n");
    if (!config_process_assignments()) {
        log_error("Error while processing variable assignments.\n");
        scanparse = FALSE;
    }

    log_info(LOG_INFO, "Processing variable type extensions...\n");
    if (!regexp_process_extensions()) {
        log_error("Error while processing variable type extensions.\n");
        scanparse = FALSE;
    }

    if (set_arch() != OK) {
        log_error("Unable to read fli4l architecture.\n");
        scanparse = FALSE;
    }
    else {
        log_info (LOG_INFO, "architecture: %s\n", var_read("ARCH"));
    }

    log_info(LOG_INFO, "Compiling variable types...\n");
    if (!vartype_finalize_type_system()) {
        log_error("Error while compiling variable types.\n");
        scanparse = FALSE;
    }

    log_info(LOG_INFO, "Checking variable values...\n");
    if (!var_check_values()) {
        log_error("Error while checking variable values.\n");
        scanparse = FALSE;
    }

    if (!scanparse) {
        fatal_exit("Error while processing configuration, aborting!\n");
    }

    dec_log_indent_level();
    log_info(LOG_INFO, "Finished processing configuration.\n");

    log_info(LOG_INFO, "Checking variables...\n");
    if (check_all_variables() != OK) {
        fatal_exit("Error in configuration, aborting!\n");
    }

    if (is_mkfli4l && !check_only) {
        /* ignore package list if there is no KERNEL_VERSION defined */
        kernel_version = var_read("KERNEL_VERSION");
        if (kernel_version) {
            log_info(LOG_INFO, "Generating zip list...\n");
            inc_log_indent_level();
            res = make_zip_list(kernel_version);
            dec_log_indent_level();
        }
        else {
            fatal_exit("Undefined KERNEL_VERSION, "
                       "unable to generate archives.\n");
        }
    }

    log_info (LOG_INFO, "Executing extended check scripts...\n");
    if (!execute_all_extended_checks() || res != OK) {
        fatal_exit ("Error in configuration, aborting...\n");
    }

    if (is_mkfli4l && !check_only) {
        char const *boottype;

        log_info (LOG_INFO, "Preparing archives...\n");

        boottype = var_read(VAR_BOOT_TYPE);
        if (boottype == NULL) {
            fatal_exit(VAR_BOOT_TYPE " must be defined!\n");
        }

        if (resolve_dependencies(kernel_version) != OK) {
            fatal_exit("Unable to resolve dependecies!\n");
        }

        if (resolve_library_dependencies() != OK) {
            fatal_exit("Unable to resolve library dependecies!\n");
        }

        if (create_library_symlinks() != OK) {
            fatal_exit("Unable to create library symlinks!\n");
        }

        if (create_build_info() != OK) {
            fatal_exit("Unable to create build information!\n");
        }

        if (determine_rootfs_compression(kernel_version) != OK) {
            fatal_exit("Unable to determine ROOTFS compression!\n");
        }

        if (!dump_variables(rc_file)) {
            fatal_exit("Error while writing %s (error in variables)!\n", rc_file);
        }

        if (add_boot_type_specific_entries(boottype) != OK) {
            fatal_exit("Unable to perform actions dependent on the boot type!\n");
        }

        if (prepare_zip_lists() != OK) {
            fatal_exit("Error while preparing zip lists!\n");
        }

        log_info (LOG_INFO, "Creating archives...\n");

        if (create_archives(boottype) != OK) {
            fatal_exit("Error while creating archives!\n");
        }

#ifdef SKIP_SYSLINUX
        if (!access(syslinux_template_file, R_OK)) {
#endif
            log_info(LOG_INFO, "generating syslinux.cfg...\n");
            inc_log_indent_level();
            if (mk_syslinux(syslinux_template_file, syslinux_cfg_file) != OK) {
                fatal_exit("Error building syslinux.cfg, aborting...\n");
            }
            dec_log_indent_level();
            log_info(LOG_INFO, "\nfinished generating syslinux.cfg\n");
#ifdef SKIP_SYSLINUX
        } else {
            log_error("-> skipping generation of syslinux.cfg due to missing template file\n");
        }
#endif
    }

    log_info (LOG_INFO, "Successs.\n");

    close_logfile();

    regexp_done();

    var_fini_module();
    vartype_fini_module();
    package_fini_module();

    return 0;
}
