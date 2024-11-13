/*****************************************************************************
 *  @file config.c
 *  Functions for processing config/<package>.txt files.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/var.h>
#include <libmkfli4l/regex.h>
#include <libmkfli4l/check.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/file.h>
#include <libmkfli4l/options.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/file.h>
#include <ziplist/opt.h>
#include <vartype.h>
#include <var.h>
#include "config.h"

/**
 * Name of a superseding file.
 */
#define SUPERSEDES "_fli4l"

/**
 * Describes a file handler.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The package file being processed.
 * @param supersedes
 *  TRUE if it is a superseding file, FALSE otherwise. This is currently
 *  only used when processing configuration files.
 * @return
 *  TRUE if processing the package file was successful, FALSE otherwise.
 */
typedef BOOL (*file_handler_t)(
    struct package_t *package,
    struct package_file_t *file,
    BOOL supersedes
);

/**
 * Processes a (new-style) configuration file. Old-style configuration files
 * (config.txt) have not been supported for ages.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The configuration file being processed.
 * @param supersedes
 *  TRUE if it is a superseding configuration file, FALSE otherwise.
 *  Superseding configuration files use the VAR_PRIORITY_SUPERSEDE
 *  priority, all other files use the VAR_PRIORITY_CONFIGURATION priority
 *  when a variable is being set.
 * @return
 *  TRUE if processing the configuration file was successful, FALSE
 *  otherwise.
 */
static BOOL
handle_config_file(
    struct package_t *package,
    struct package_file_t *file,
    BOOL supersedes
)
{
    int const priority =
        supersedes ? VAR_PRIORITY_SUPERSEDE : VAR_PRIORITY_CONFIGURATION;
    char const *pkg_name = package_get_name(package);

    if (strcasecmp(pkg_name, "config") == 0) {
        log_error("Error: %s is an old style configuration file."
                  " READ DOCUMENTATION!\n", package_file_get_name(file));
        return FALSE;
    }

    log_info(
        LOG_VERBOSE,
        "Reading variable values for package %s...\n",
        pkg_name
    );

    read_config_file(file, package, priority);
    return TRUE;
}

/**
 * Processes a variable type file.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The variable type file being processed.
 * @param supersedes
 *  Not used.
 * @return
 *  TRUE if processing the variable type file was successful, FALSE
 *  otherwise.
 */
static BOOL
handle_vartype_file(
    struct package_t *package,
    struct package_file_t *file,
    BOOL supersedes
)
{
    log_info(
        LOG_VERBOSE,
        "Reading variable types for package %s...\n",
        package_get_name(package)
    );

    regexp_read_file(file, package);
    return TRUE;

    (void) supersedes;
}

/**
 * Processes a variable definition file.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The variable definition file being processed.
 * @param supersedes
 *  Not used.
 * @return
 *  TRUE if processing the variable definition file was successful, FALSE
 *  otherwise.
 */
static BOOL
handle_vardef_file(
    struct package_t *package,
    struct package_file_t *file,
    BOOL supersedes
)
{
    log_info(
        LOG_VERBOSE,
        "Reading variable definitions for package %s...\n",
        package_get_name(package)
    );

    read_check_file(file, package);
    return TRUE;

    (void) supersedes;
}

/**
 * Processes an extended check script.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The extended check script being processed.
 * @param supersedes
 *  Not used.
 * @return
 *  TRUE if processing the extended check script was successful, FALSE
 *  otherwise.
 */
static BOOL
handle_extcheck_file(
    struct package_t *package,
    struct package_file_t *file,
    BOOL supersedes
)
{
    log_info(
        LOG_VERBOSE,
        "Reading extended check script for package %s...\n",
        package_get_name(package)
    );

    return parse_check_file(file, package);
    UNUSED(supersedes);
}

/**
 * Maps a package file type to the extension it uses.
 *
 * @param type
 *  The package file type.
 * @return
 *  A pointer to a statically allocated string containing the extension
 *  associated with the package file type.
 */
static char const *
map_type_to_ext(int type)
{
    switch (type) {
    case PACKAGE_FILE_TYPE_VARTYPE :
        return def_regex_ext;
    case PACKAGE_FILE_TYPE_VARDEF :
        return def_check_ext;
    case PACKAGE_FILE_TYPE_CONFIG :
        return def_cfg_ext;
    case PACKAGE_FILE_TYPE_IMAGE :
        return def_opt_ext;
    case PACKAGE_FILE_TYPE_EXTCHECK :
        return def_extcheck_ext;
    default :
        return "";
    }
}

/**
 * Maps a package file type to a descriptive string.
 *
 * @param type
 *  The package file type.
 * @return
 *  A pointer to a statically allocated string containing a human-readable
 *  description of the package file type.
 */
static char const *
map_type_to_desc(int type)
{
    switch (type) {
    case PACKAGE_FILE_TYPE_VARTYPE :
        return "variable types";
    case PACKAGE_FILE_TYPE_VARDEF :
        return "variable definitions";
    case PACKAGE_FILE_TYPE_CONFIG :
        return "variable values";
    case PACKAGE_FILE_TYPE_IMAGE :
        return "image actions";
    case PACKAGE_FILE_TYPE_EXTCHECK :
        return "extended check script";
    default :
        return "";
    }
}

/**
 * Maps a package file type to a function handling files of this type.
 *
 * @param type
 *  The package file type.
 * @return
 *  A pointer to a function handling files of this type.
 */
static file_handler_t
map_type_to_handler(int type)
{
    switch (type) {
    case PACKAGE_FILE_TYPE_VARTYPE :
        return &handle_vartype_file;
    case PACKAGE_FILE_TYPE_VARDEF :
        return &handle_vardef_file;
    case PACKAGE_FILE_TYPE_CONFIG :
        return &handle_config_file;
    case PACKAGE_FILE_TYPE_EXTCHECK :
        return &handle_extcheck_file;
    default :
        return NULL;
    }
}

/**
 * Processes a package file by invoking the correct package file handler on it.
 *
 * @param dir
 *  The directory the file lives in.
 * @param pkg
 *  The name of the package this file belongs to.
 * @param type
 *  The type of the package file to be handled.
 * @param supersedes
 *  TRUE if it is a superseding file, FALSE otherwise. This is currently
 *  only used when processing configuration files.
 * @return
 *  TRUE if processing the package file was successful, FALSE otherwise.
 */
static BOOL
handle_file(char const *dir, char const *pkg, int type, BOOL supersedes)
{
    struct package_t *package;
    struct package_file_t *file;
    char const *const ext = map_type_to_ext(type);
    int const dir_len = strlen(dir);
    int const pkg_len = strlen(pkg);
    int const ext_len = strlen(ext);
    file_handler_t const handler = map_type_to_handler(type);
    char *name;
    BOOL result = TRUE;

    name = (char *)
        safe_malloc(dir_len + 1 + pkg_len + ext_len + 1);
    strcpy(name, dir);
    strcat(name, "/");
    strcat(name, pkg);
    strcat(name, ext);

    package = package_get_or_add(pkg);
    file = package_set_file(package, type, name);

    if (handler) {
        log_info(LOG_INFO, "Processing %s in %s\n", map_type_to_desc(type), pkg);

        result = (*handler)(package, file, supersedes);
    }

    free(name);
    return result;
}

/**
 * Processes a directory of package files by invoking handle_file() on each of
 * its files matching a given package file type.
 *
 * @param dir
 *  The directory to be processed.
 * @param type
 *  The type of the package file to be handled.
 * @param packages
 *  An array of package names that are to be processed (only). If the array
 *  is empty, all matching package files (and an optionally existing
 *  superseding file) are processed.
 * @param num_packages
 *  The number of entries in the 'packages' array.
 * @return
 *  TRUE if processing all matching package files in the directory was
 *  successful, FALSE otherwise.
 */
static BOOL
process_directory(char const *dir, int type, char **packages, int num_packages)
{
    DIR *dirp;
    struct dirent *dp;
    char const *const ext = map_type_to_ext(type);
    int const ext_len = strlen(ext);
    BOOL supersedes = FALSE;
    BOOL result = TRUE;

    log_info(LOG_INFO, "Reading %s...\n", map_type_to_desc(type));
    inc_log_indent_level();

    dirp = opendir(dir);
    if (!dirp) {
        log_error(
            "Error opening directory '%s': %s\n", config_dir, strerror(errno)
        );
        result = FALSE;
    }
    else {
        while ((dp = readdir(dirp)) != NULL) {
            int const len = strlen(dp->d_name);
            if (len > ext_len
                    && strcasecmp(dp->d_name + len - ext_len, ext) == 0) {
                dp->d_name[len - ext_len] = '\0';
                if (strcmp(dp->d_name, SUPERSEDES) == 0) {
                    supersedes = TRUE;
                }
                else {
                    char *name = dp->d_name;
                    if (num_packages == 0
                            || (type == PACKAGE_FILE_TYPE_VARTYPE && strcasecmp(dp->d_name, PKG_BASE) == 0)
                            || bsearch(&name, packages, num_packages, sizeof(char *), &cmp_package) != NULL) {
                        if (!handle_file(dir, dp->d_name, type, FALSE)) {
                            result = FALSE;
                        }
                    }
                }
            }
        }

        (void) closedir(dirp);

        if (supersedes && type == PACKAGE_FILE_TYPE_CONFIG && num_packages == 0) {
            if (!handle_file(dir, SUPERSEDES, type, TRUE)) {
                result = FALSE;
            }
        }
    }

    dec_log_indent_level();
    return result;
}

BOOL
read_config(char const *config_dir, char const *check_dir, char **packages)
{
    BOOL result = TRUE;

    int num_packages = 0;
    if (packages) {
        char **p = packages;
        while (*p++) {
            ++num_packages;
        }
    }

    if (!process_directory(check_dir, PACKAGE_FILE_TYPE_VARTYPE, packages, num_packages)) {
        result = FALSE;
    }
    if (!process_directory(check_dir, PACKAGE_FILE_TYPE_VARDEF, packages, num_packages)) {
        result = FALSE;
    }
    if (!process_directory(check_dir, PACKAGE_FILE_TYPE_EXTCHECK, packages, num_packages)) {
        result = FALSE;
    }
    if (!process_directory(config_dir, PACKAGE_FILE_TYPE_CONFIG, packages, num_packages)) {
        result = FALSE;
    }
    if (!process_directory("opt", PACKAGE_FILE_TYPE_IMAGE, packages, num_packages)) {
        result = FALSE;
    }

    var_add_weak_declaration("base", "CONFIG_DIR", config_dir_with_slashes,
                             "internal variable", NULL, TYPE_UNKNOWN,
                             __FILE__, __LINE__, 0, PACKAGE_FILE_TYPE_VARTYPE);

    return result;
}
