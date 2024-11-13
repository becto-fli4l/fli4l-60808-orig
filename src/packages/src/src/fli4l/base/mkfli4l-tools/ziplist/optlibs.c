/*****************************************************************************
 *  @file optlibs.c
 *  Functions for resolving library dependencies.
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
 *****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>

#include <libmkfli4l/defs.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/str.h>
#include <elfinfo/readelf.h>
#include "optlibs.h"
#include "opt.h"

/**
 * The number of library directories to allocate in one go.
 */
#define LIBDIR_ARRAY_SIZE 8
/**
 * The number of library entries per library directory to allocate in one go.
 */
#define LIB_ARRAY_SIZE 64

/**
 * Describes an entry in a library directory.
 */
struct lib_t {
    /**
     * The file name of the library without a path.
     */
    char *name;
    /**
     * The SONAME of the library. If the library does not possess a SONAME,
     * the library's base name is stored.
     */
    char *soname;
};

/**
 * Describes a library directory entry in the library cache.
 */
struct libdir_t {
    /**
     * Represents the library directory as used on the target file system, e.g.
     * /usr/lib.
     */
    char *path;
    /**
     * Contains the path to the library directory on the local file system,
     * e.g. opt/usr/lib.
     */
    char *dirname;
    /**
     * TRUE if the directory is part of the global library search path, FALSE
     * otherwise.
     */
    BOOL global;
    /**
     * The libraries being part of this library directory.
     */
    array_t *libraries;
};

/**
 * Describes an entry in a dependency chain.
 */
struct dep_entry_t {
    /**
     * The entry within a dependency chain.
     */
    struct entry_t *entry;
    /**
     * The parent entry. May be NULL only for the first entry in the chain.
     */
    struct dep_entry_t *parent;
};


/**
 * A list of library directories that will be searched for libraries.
 */
static array_t *libdir_array;

/**
 * Searches a library directory and associates SONAMEs with the libraries
 * contained therein.
 *
 * @param path
 *  The path to the library directory as used on the target file system, e.g.
 *  /usr/lib.
 * @param global
 *  TRUE if the path is to be added to the global library search path, FALSE
 *  otherwise.
 * @return
 *  The entry describing the library directory.
 */
static struct libdir_t *
ldconfig(char const *path, BOOL global)
{
    DIR *dirp;
    struct dirent *dp;
    char dirname[BUF_SIZE];
    struct libdir_t *entry;

    sprintf(dirname, "opt%s", path);

    entry = (struct libdir_t *) safe_malloc(sizeof(struct libdir_t));
    entry->path = strsave(path);
    entry->dirname = strsave(dirname);
    entry->global = global;
    entry->libraries = init_array(LIB_ARRAY_SIZE);
    append_new_elem(libdir_array, entry);

    log_info (LOG_LIB_DEP, "adding library directory %s to %s search path\n", path, global ? "global" : "local");
    inc_log_indent_level();

    dirp = opendir(dirname);
    if (dirp) {
        while ((dp = readdir(dirp)) != 0) {
            struct readelf_descriptor *desc;
            char libname[BUF_SIZE];
            sprintf(libname, "%s/%s", dirname, dp->d_name);
            if (readelf_process_file(libname, &desc) == 0) {
                struct lib_t *lib
                    = (struct lib_t *) safe_malloc(sizeof(struct lib_t));
                lib->name = strsave(dp->d_name);
                lib->soname = desc->soname ? strsave(desc->soname) : strsave(dp->d_name);
                append_new_elem(entry->libraries, lib);
                log_info (LOG_LIB_DEP, "adding library %s [SONAME %s]\n", libname, lib->soname);
                readelf_free_descriptor(desc);
            }
        }

        closedir(dirp);
    }

    dec_log_indent_level();
    return entry;
}

/**
 * Searches for a library directory entry matching passed "path".
 *
 * @param path
 *  The path to check.
 * @return
 *  A pointer to the corresponding library directory entry or NULL if no such
 *  entry has been found.
 */
static struct libdir_t *
lookup_libdir_entry(char const *path)
{
    ARRAY_ITER(it_dir, libdir_array, dir, struct libdir_t) {
        if (strcmp(path, dir->path) == 0) {
            return dir;
        }
    }

    return NULL;
}

/**
 * Searches for a SONAME in the archive entry list given a library directory.
 * @param soname
 *  The SONAME to be looked up.
 * @param libdir
 *  The library directory to prepend.
 * @return
 *  The archive list entry if found or NULL otherwise.
 */
static struct entry_t *
get_zip_list_entry_by_soname(char const *soname, char const *libdir)
{
    int const len = strlen(libdir) - 1;
    ARRAY_ITER(array_iter, files_array, p, struct entry_t)
    {
        if (p->soname && strcmp(soname, p->soname) == 0) {
            if (strncmp(p->target, libdir + 1, len) == 0
                    && p->target[len] == '/'
                    && strchr(p->target + len + 1, '/') == NULL) {
                return p;
            }
        }
    }

    return NULL;
}

/**
 * Tries to resolve a library dependency by some archive list entry. First, the
 * dependency is looked up in the directory specified by "rpath" (if any). If
 * this fails, the dependency is looked up in /lib and finally in /usr/lib.
 *
 * @param dep
 *  The ELF dependency entry to be resolved.
 * @param rpath
 *  An optional RPATH to consider. May be NULL.
 * @return
 *  TRUE if the dependency has been resolved by some archive list entry, FALSE
 *  otherwise.
 */
static BOOL
try_to_resolve_dependency(struct elf_dep_t *dep, char const *rpath)
{
    if (!dep->file && rpath) {
        dep->file = get_zip_list_entry_by_soname(dep->soname, rpath);
    }
    if (!dep->file) {
        dep->file = get_zip_list_entry_by_soname(dep->soname, "/lib");
    }
    if (!dep->file) {
        dep->file = get_zip_list_entry_by_soname(dep->soname, "/usr/lib");
    }

    /*
     * We also support dependency resolution by explicit symlink. This is to
     * catch situations where a library has bumped its version number due to
     * "political" issues (consider e.g. uClibc --> libc.so.0, uClibc-ng -->
     * libc.so.1). As this requirement is very special and need not be
     * generalized unnecessarily, we don't support RPATHs but only the
     * hard-coded library search paths /lib and /usr/lib.
     */
    if (!dep->file) {
        char *full_name;
        struct entry_t *entry;

        full_name = strcat_save(strsave("usr/lib/"), dep->soname);
        entry = get_zip_list_target_entry(full_name);
        free(full_name);
        if (entry && entry->entry_type == ENTRY_SYMLINK) {
            dep->file = entry;
            return TRUE;
        }

        full_name = strcat_save(strsave("lib/"), dep->soname);
        entry = get_zip_list_target_entry(full_name);
        free(full_name);
        if (entry && entry->entry_type == ENTRY_SYMLINK) {
            dep->file = entry;
            return TRUE;
        }

        return FALSE;
    }
    else {
        return TRUE;
    }
}

/**
 * Builds a stringized dependency chain from a dependency entry.
 *
 * @param depender
 *   The dependency entry arrived at.
 * @return
 *   A string representing the dependency chain from the root to the depender.
 */
static char *
create_dep_chain(struct dep_entry_t *depender)
{
    char *dep_chain = NULL;
    struct dep_entry_t *dep_entry = depender;
    while (dep_entry) {
        char *new_chain = strsave(dep_entry->entry->soname);
        if (dep_chain) {
            new_chain = strcat_save(new_chain, "-->");
            new_chain = strcat_save(new_chain, dep_chain);
            free(dep_chain);
            dep_chain = new_chain;
        }
        else {
            dep_chain = new_chain;
        }
        dep_entry = dep_entry->parent;
    }
    return dep_chain;
}

/**
 * Searches for a SONAME in a given library directory and adds it to the
 * archive entry list if successful.
 *
 * @param root
 *  The ELF module representing the dependency root (which may have an RPATH).
 * @param depender
 *  The entry depending on the SONAME to be looked up.
 * @param soname
 *  The SONAME to be looked up.
 * @param dir
 *  The library directory to search.
 * @param dep_opts
 *  The entry options to use for the archive list entry if the SONAME can be
 *  found.
 * @retval OK if the entry has been found and added successfully.
 * @retval 1 if the entry has not been found and the lookup should continue in
 *  another library directory (if possible).
 * @retval ERR if adding the library entry has failed and the lookup should be
 *  aborted.
 */
static int
search_and_add_dependency(struct entry_t *root, struct dep_entry_t *depender, char const *soname, struct libdir_t *dir, array_t *dep_opts)
{
    ARRAY_ITER(it_lib, dir->libraries, lib, struct lib_t) {
        if (strcmp(soname, lib->soname) == 0) {
            char *dep_chain;
            struct entry_t *result;
            int const len = strlen(dir->dirname) + 1 + strlen(lib->name) + 1;
            char *libname = (char *) safe_malloc(len);
            strcpy(libname, dir->dirname);
            strcat(libname, "/");
            strcat(libname, lib->name);

            dep_chain = create_dep_chain(depender);
            log_info(LOG_LIB_DEP, "%s:%s: dependency to %s satisfied by %s\n", get_location(root->ziptgt), dep_chain, soname, libname);
            free(dep_chain);
            result = add_zip_list_entry(libname, ZIP_SRC_OPT, root->ziptgt, ENTRY_FILE, dep_opts, "");
            free(libname);

            if (result != NULL) {
                result->secondary = TRUE;
                return OK;
            }
            else {
                return ERR;
            }
        }
    }

    return 1;
}

/**
 * Searches for a SONAME in a set of suitable directories and adds it to the
 * archive entry list if successful. First, the RPATH of the depender is tried
 * (if available), then /lib and last but not least /usr/lib.
 *
 * @param root
 *  The ELF module representing the dependency root (which may have an RPATH).
 * @param depender
 *  The entry depending on the SONAME to be looked up.
 * @param soname
 *  The SONAME to be looked up.
 * @return
 *  TRUE if successful, FALSE otherwise.
 * @note
 *  Currently, there is no support for handling DT_RUNPATH entries in ELF
 *  files, simply because they are not used in any binary belonging to the
 *  fli4l image.
 */
static BOOL
add_dependency(struct entry_t *root, struct dep_entry_t *depender, char const *soname)
{
    DECLARE_ARRAY_ITER(it_dir, dir, struct libdir_t);
    array_t *const dep_opts = parse_entry_options("mode=555");

    /* process RPATH if available */
    if (root->rpath) {
        int res;
        struct libdir_t *dir = lookup_libdir_entry(root->rpath);
        if (!dir) {
            dir = ldconfig(root->rpath, FALSE);
        }

        res = search_and_add_dependency(root, depender, soname, dir, dep_opts);
        if (res < 0) {
            return FALSE;
        }
        else if (res == OK) {
            return TRUE;
        }
    }

    /* process global library search path */
    ARRAY_ITER_LOOP(it_dir, libdir_array, dir) {
        if (dir->global) {
            int res = search_and_add_dependency(root, depender, soname, dir, dep_opts);
            if (res < 0) {
                return FALSE;
            }
            else if (res == OK) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 * Resolves a library dependency by some archive list entry. First, the
 * dependency is looked up in the directory specified by "rpath" (if any). If
 * this fails, the dependency is looked up in /lib and finally in /usr/lib.
 *
 * @param root
 *  The ELF module representing the dependency root (which may have an RPATH).
 * @param depender
 *  The ELF dependency entry to be resolved.
 * @return
 *  TRUE if the dependency has been resolved by some archive list entry, FALSE
 *  otherwise.
 */
static BOOL
resolve_dependencies(struct entry_t *root, struct dep_entry_t *depender)
{
    BOOL result = TRUE;
    int i;
    for (i = 0; i < depender->entry->num_needed; ++i) {
        struct elf_dep_t *dep = &depender->entry->needed[i];
        if (try_to_resolve_dependency(dep, root->rpath) ||
                (add_dependency(root, depender, dep->soname) &&
                try_to_resolve_dependency(dep, root->rpath))) {
            struct dep_entry_t *dep_entry;

            if (result && root->ziptgt < dep->file->ziptgt) {
                char const *oldtype = get_location(dep->file->ziptgt);
                dep->file->ziptgt = root->ziptgt;
                log_info (LOG_LIB_DEP, "moving %s from %s to %s as referenced by %s\n",
                          dep->file->target,
                          oldtype,
                          get_location(dep->file->ziptgt),
                          root->target);
            }

            dep_entry = (struct dep_entry_t *)
                    safe_malloc(sizeof(struct dep_entry_t));
            dep_entry->parent = depender;
            dep_entry->entry = dep->file;

            if (!resolve_dependencies(root, dep_entry)) {
                result = FALSE;
            }

            free(dep_entry);
        }
        else {
            char *dep_chain = create_dep_chain(depender);
            log_error("  %s:%s depends on %s which is missing\n", get_location(root->ziptgt), dep_chain, dep->soname);
            free(dep_chain);
            result = FALSE;
        }
    }

    return result;
}

int
resolve_library_dependencies(void)
{
    int result = OK;
    DECLARE_ARRAY_ITER(array_iter, entry, struct entry_t);

    log_info (LOG_INFO, "resolving library dependencies\n");

    libdir_array = init_array(LIBDIR_ARRAY_SIZE);
    ldconfig("/lib", TRUE);
    ldconfig("/usr/lib", TRUE);

    ARRAY_ITER_LOOP(array_iter, files_array, entry)
    {
        /* process only primary ELF modules */
        if (entry->soname && !entry->secondary) {
            struct dep_entry_t *dep_entry = (struct dep_entry_t *)
                    safe_malloc(sizeof(struct dep_entry_t));
            dep_entry->parent = NULL;
            dep_entry->entry = entry;

            if (!resolve_dependencies(entry, dep_entry)) {
                result = ERR;
            }

            free(dep_entry);
        }
    }

    log_info (LOG_LIB_DEP, "resolving library dependencies: %s\n", result == OK ? "SUCCESSFUL" : "FAILED");
    return result;
}

int
create_library_symlinks(void)
{
    DECLARE_ARRAY_ITER(array_iter, p, struct entry_t);
    char const linkopts_prefix[] = "linktarget=";
    int linkopts_prefixlen = sizeof(linkopts_prefix);
    int result = OK;

    log_info (LOG_INFO, "creating library symlinks\n");
    inc_log_indent_level();

    ARRAY_ITER_LOOP(array_iter, files_array, p)
    {
        if (p->soname != NULL) {
            char *dir = strsave(p->target);
            char *base;
            char *q = strrchr(dir, '/');
            if (q != NULL) {
                base = strsave(++q);
                *q = '\0';
            }
            else {
                base = strsave(dir);
                *dir = '\0';
            }

            if (strcmp(base, p->soname) != 0) {
                char *linkname;
                char *linkopts;
                int linkopts_len = linkopts_prefixlen;

                int const namelen = strlen(dir) + strlen(p->soname) + 1;
                linkname = (char *) safe_malloc(namelen);
                strcpy(linkname, dir);
                strcat(linkname, p->soname);

                linkopts_len += strlen(base);
                linkopts = (char *) safe_malloc(linkopts_len);
                strcpy(linkopts, linkopts_prefix);
                strcat(linkopts, base);

                log_info (LOG_LIB_SYMLINK, "adding library symlink from %s to %s\n", linkname, base);
                result = add_zip_list_entry(linkname, p->zipsrc, p->ziptgt, ENTRY_SYMLINK, parse_entry_options(linkopts), "") != NULL
                            ? OK : ERR;
                free(linkname);
                free(linkopts);
            }

            free(dir);
            free(base);

            if (result != OK) {
                break;
            }
        }
    }

    dec_log_indent_level();
    return result;
}
