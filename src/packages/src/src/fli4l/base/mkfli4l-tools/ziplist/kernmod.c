/*****************************************************************************
 *  @file kernmod.c
 *  Functions for resolving kernel module dependencies.
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
#include <ctype.h>
#include <string.h>
#include <regex.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/array.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/options.h>
#include <libmkfli4l/parse.h>
#include "kernmod.h"
#include "opt.h"

#define DEP_BUFFER_SIZE 4096

/**
 * The number of kernel module entries to allocate in one go.
 */
#define MODULE_ARRAY_SIZE 1024
/**
 * The number of kernel module dependencies to allocate in one go.
 */
#define MODULE_DEP_ARRAY_SIZE 16
/**
 * The number of alias entries to allocate in one go.
 */
#define MODULE_ALIAS_ARRAY_SIZE 16
/**
 * The suffix used for kernel modules.
 */
#define MODULE_SUFFIX ".ko"

/**
 * Describes a kernel module.
 */
struct module_t {
    /**
     * The base name of the module without path and suffix.
     */
    char *name;
    /**
     * The path to the module.
     */
    char *path;
    /**
     * A list of pointers to module_t objects that constitute the module's
     * dependencies.
     */
    array_t *deps;
    /**
     * A list of alias names for this module.
     */
    array_t *aliases;
    /**
     * The target archive where to include this module.
     */
    int ziptgt;
};

/**
 * Describes a kernel module alias.
 */
struct alias_t {
    /**
     * The module alias (i.e. without path and suffix).
     */
    char *name;
};

/**
 * A list of all modules found in the modules.dep.
 */
static array_t *module_array;

/**
 * Compares two module names, ignoring differences wrt. hyphen/underscore.
 *
 * @param m1
 *  The first module name.
 * @param m2
 *  The second module name.
 * @return
 *  TRUE if the module names are equal, FALSE otherwise.
 */
static BOOL
are_modules_equal(char const *m1, char const *m2)
{
    while (*m1 && *m2) {
        char const c1 = *m1 == '-' ? '_' : *m1;
        char const c2 = *m2 == '-' ? '_' : *m2;
        if (c1 != c2) {
            return FALSE;
        }
        ++m1;
        ++m2;
    }
    return *m1 == *m2;
}

/**
 * Prepends "/lib/modules/<kernel-version>/" prefix to a module path from
 * modules.dep if necessary. Since module-init-tools 3.6, depmod produces
 * shorter module paths without this prefix. This function adds that prefix if
 * necessary.
 *
 * @param path
 *  A module path as read from modules.dep.
 * @param prefix
 *  The /lib/... prefix to prepend if necessary.
 * @return
 *  The resulting module path.
 */
static char *
extend_relative_module_path(char const *path, char const *prefix)
{
    int const prefix_len = strlen(prefix);
    if (strncmp(path, prefix, prefix_len) == 0) {
        return strsave(path);
    } else {
        char *result = (char *) safe_malloc(prefix_len + strlen(path) + 1);
        strcpy(result, prefix);
        strcat(result, path);
        return result;
    }
}

/**
 * Finds or creates a module_t object belonging to a module path as found in
 * modules.dep. If the module_t object is created, it is added to the global
 * module list before being returned.
 *
 * @param path
 *  A module path as read from modules.dep.
 * @param prefix
 *  The /lib/... prefix to prepend if necessary.
 * @return
 *  The resulting module_t object or NULL if the list contains is already a
 *  module with the same name but with a different path.
 * @warning
 *  This function modifies "path"!
 */
static struct module_t *
find_or_create_module(char *path, char const *prefix)
{
    char *suffix_start;
    char *full_path;
    char *module;
    struct module_t *entry;
    DECLARE_ARRAY_ITER(module_array_iter, m, struct module_t);

    full_path = extend_relative_module_path(path, prefix);

    suffix_start = strrchr(path, '.');
    if (suffix_start && strcmp(suffix_start, MODULE_SUFFIX) == 0) {
        *suffix_start = '\0';
    }
    module = strrchr(path, '/');
    module = module ? module + 1 : path;

    ARRAY_ITER_LOOP(module_array_iter, module_array, m) {
        if (are_modules_equal(module, m->name)) {
            if (strcmp(full_path, m->path) == 0) {
                free(full_path);
                return m;
            }
            else {
                log_error("module '%s' is located at two different paths '%s' and '%s'\n",
                          module, m->path, full_path);
                return NULL;
            }
        }
    }

    entry = (struct module_t *) safe_malloc(sizeof(struct module_t));
    entry->name = strsave(module);
    entry->path = full_path;
    entry->deps = init_array(MODULE_DEP_ARRAY_SIZE);
    entry->aliases = init_array(MODULE_ALIAS_ARRAY_SIZE);
    entry->ziptgt = ZIP_TGT_UNKNOWN;
    append_new_elem(module_array, entry);

    return entry;
}

/**
 * Tries to find a module by name in the global module list. Aliases are not
 * considered.
 *
 * @param name
 *  The module name.
 * @return
 *  The module_t object belonging to the module or NULL if not found.
 */
static struct module_t *
lookup_module(char const *name)
{
    ARRAY_ITER(module_iter, module_array, m, struct module_t) {
        if (are_modules_equal(m->name, name)) {
            return m;
        }
    }

    return NULL;
}

/**
 * Reads a modules.dep file and creates module_t objects for each module read.
 * Module dependencies are recognized and stored at the same time.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
read_modules_dependency_file(char const *kernel_version)
{
    char buffer [DEP_BUFFER_SIZE];
    FILE *f;
    char prefix [BUF_SIZE];
    int res = OK;

    module_array = init_array(MODULE_ARRAY_SIZE);

    snprintf(prefix, sizeof prefix, "/lib/modules/%s/", kernel_version);

    sprintf(buffer, "opt%smodules.dep", prefix);
    f = fopen(buffer, "r");
    if (!f) {
        log_error("unable to open '%s' containing module dependencies\n", buffer);
        return ERR;
    }

    while (fgets(buffer, sizeof(buffer) - 1, f) != NULL) {
        char *end = strchr(buffer, ':');
        if (end) {
            struct module_t *module;
            char *deps = end + 1;

            *end = '\0';
            module = find_or_create_module(buffer, prefix);
            if (module) {
                while (*deps) {
                    if (!isspace((int) *deps)) {
                        char *dep_name = deps;
                        char orig;
                        struct module_t *dep;

                        while (*deps && !isspace((int) *deps))
                            ++deps;
                        orig = *deps;
                        *deps = '\0';

                        dep = find_or_create_module(dep_name, prefix);
                        if (!dep) {
                            res = ERR;
                        }
                        else {
                            append_new_elem(module->deps, dep);
                        }
                        *deps = orig;
                    }
                    else {
                        ++deps;
                    }
                }
            }
            else {
                res = ERR;
            }
        }
    }
    fclose(f);

    return res;
}

/**
 * Reads a module alias file like modules.alias and associates alias_t objects
 * with the corresponding module for each module alias read.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @param file
 *  The file to process.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
read_module_alias_file(char const *kernel_version, char const *file)
{
    char buffer[BUF_SIZE];
    FILE *f;

    sprintf(buffer, "opt/lib/modules/%s/%s", kernel_version, file);
    f = fopen(buffer, "r");
    if (!f) {
        log_error("unable to open '%s' containing module aliases\n", buffer);
        return ERR;
    }

    while (fgets(buffer, sizeof(buffer) - 1, f) != NULL) {
        /*
         * kristov: cannot use sscanf("alias %s %s", ...) as some aliases
         * in the 2.6.x kernel contain spaces (acerhdf)
         */
        char const prefix[] = "alias ";
        if (strncmp(buffer, prefix, sizeof prefix - 1) == 0) {
            char *alias = buffer + sizeof prefix - 1;
            char *p = strrchr(alias, ' ');
            if (p != NULL) {
                struct alias_t *entry;
                struct module_t *module;
                char *name = p + 1;
                *p = '\0';
                p = strchr(name, '\n');
                if (p != NULL) {
                    *p = '\0';
                }

                module = lookup_module(name);
                if (!module) {
                    log_error("alias '%s' references unknown module '%s'\n", alias, name);
                    return ERR;
                }

                entry = (struct alias_t *) safe_malloc(sizeof(struct alias_t));
                entry->name = strsave(alias);
                append_new_elem(module->aliases, entry);
            }
        }
    }
    fclose(f);

    return OK;
}

/**
 * Reads all module alias files and associates alias_t objects with the
 * corresponding module for each module alias read.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
read_module_alias_files(char const *kernel_version)
{
    int res;
    if ((res = read_module_alias_file(kernel_version, "modules.alias")) != OK) {
        return res;
    }
    if ((res = read_module_alias_file(kernel_version, "modules.symbols")) != OK) {
        return res;
    }
    return OK;
}

/**
 * Computes recursively all modules that are needed in the target archive. This
 * encompasses modules that are not explicitly selected but are direct or
 * indirect dependencies of selected modules.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
compute_modules_used(const char *kernel_version)
{
    regex_t reg;
    regmatch_t match[2];
    char expr[BUF_SIZE];
    BOOL loop;
    int res;
    DECLARE_ARRAY_ITER(entry_it, entry, struct entry_t);

    snprintf(expr, sizeof expr, "lib/modules/%s/.*/([^/.]+)\\" MODULE_SUFFIX "$", kernel_version);

    res = regcomp(&reg, expr, REG_EXTENDED);
    if (res != 0) {
        char err_buf[128];
        regerror(res, &reg, err_buf, sizeof(err_buf) - 1);
        log_error("regex error %d (%s) in %s:%d\n",
                  res, err_buf, __FILE__, __LINE__);
        return ERR;
    }

    ARRAY_ITER_LOOP(entry_it, files_array, entry) {
        if (regexec(&reg, entry->target, sizeof(match)/sizeof(regmatch_t), match,
                    0) == 0) {
            char *name = entry->target + match[1].rm_so;
            char *end = entry->target + match[1].rm_eo;
            char c = *end;
            struct module_t *module;

            *end = '\0';
            module = lookup_module(name);
            if (module) {
                module->ziptgt = entry->ziptgt;
            }
            else {
                log_error("module '%s' not part of modules.dep\n", entry->target);
                res = ERR;
            }

            *end = c;
        }
    }

    loop = TRUE;
    while (loop) {
        DECLARE_ARRAY_ITER(module_it, module, struct module_t);

        loop = FALSE;

        ARRAY_ITER_LOOP(module_it, module_array, module) {
            if (module->ziptgt != ZIP_TGT_UNKNOWN) {
                ARRAY_ITER(dep_it, module->deps, pdep, struct module_t) {
                    struct module_t *dep = pdep;
                    char const *target = get_location(module->ziptgt);
                    if (dep->ziptgt == ZIP_TGT_UNKNOWN) {
                        log_info(LOG_DEP,
                                 "added module '%s' to '%s' as dependency of module '%s'\n",
                                 dep->name, target, module->name);
                        dep->ziptgt = module->ziptgt;
                        loop = TRUE;
                    }
                    else if (module->ziptgt < dep->ziptgt) {
                        char const *source = get_location(dep->ziptgt);
                        log_info(LOG_DEP,
                                 "moving module '%s' from '%s' to '%s' due to being dependency of module '%s'\n",
                                 dep->name, source, target, module->name);
                        dep->ziptgt = module->ziptgt;
                        loop = TRUE;
                    }
                    else if (module->ziptgt > dep->ziptgt) {
                        char const *source = get_location(dep->ziptgt);
                        log_info(LOG_DEP,
                                 "not moving module '%s' from '%s' to '%s' (being dependency of module '%s')\n",
                                 dep->name, source, target, module->name);
                    }
                }
            }
        }
    }

    regfree(&reg);
    return res;
}

/**
 * Adds all modules that are selected by dependency resolution to the target
 * archive.
 *
 * @return
 *  OK on success, ERR otherwise.
 */
static int
add_missing_modules(void)
{
    int res = OK;
    ARRAY_ITER(module_it, module_array, module, struct module_t) {
        if (module->ziptgt != ZIP_TGT_UNKNOWN) {
            char filename[BUF_SIZE];
            sprintf(filename, "opt%s", module->path);
            if (add_zip_list_entry(filename, ZIP_SRC_OPT, module->ziptgt, ENTRY_FILE, NULL, "") == NULL) {
                log_error("failed to add module '%s' ('%s')\n", module->name, filename);
                res = ERR;
            }
        }
    }
    return res;
}

/**
 * Creates the module dependency file to be used on the target system.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
dump_module_dependency_file(char const *kernel_version)
{
    DECLARE_ARRAY_ITER(module_it, module, struct module_t);
    char dep_opt[BUF_SIZE];

    FILE *f = fopen(modules_dep_file, "wb");
    if (!f) {
        log_error("unable to open '%s' for writing\n", modules_dep_file);
        return ERR;
    }

    ARRAY_ITER_LOOP(module_it, module_array, module) {
        if (module->ziptgt != ZIP_TGT_UNKNOWN) {
            DECLARE_ARRAY_ITER(dep_it, pdep, struct module_t);
            fprintf(f, "%s:", module->path);
            ARRAY_ITER_LOOP(dep_it, module->deps, pdep) {
                struct module_t *dep = pdep;
                fprintf(f, " %s", dep->path);
            }
            fputc('\n', f);
        }
    }

    fclose(f);

    strcpy(dep_opt, "name=lib/modules/");
    strcat(dep_opt, kernel_version);
    strcat(dep_opt, "/modules.dep mode=600");
    return add_zip_list_entry(modules_dep_file, ZIP_SRC_OTHER, ZIP_TGT_ROOTFS,
                    ENTRY_FILE, parse_entry_options(dep_opt), "") != NULL
                        ? OK : ERR;
}

/**
 * Creates the module alias file to be used on the target system.
 *
 * @param kernel_version
 *  The version of the kernel to use.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
dump_module_alias_file(const char *kernel_version)
{
    DECLARE_ARRAY_ITER(module_it, module, struct module_t);
    char alias_opt[BUF_SIZE];

    FILE *f = fopen(modules_alias_file, "wb");
    if (!f) {
        log_error("unable to open '%s' for writing\n", modules_alias_file);
        return ERR;
    }

    ARRAY_ITER_LOOP(module_it, module_array, module) {
        if (module->ziptgt != ZIP_TGT_UNKNOWN) {
            ARRAY_ITER(alias_it, module->aliases, alias, struct alias_t) {
                fprintf(f, "alias %s %s\n", alias->name, module->name);
            }
        }
    }

    fclose(f);

    strcpy(alias_opt, "name=lib/modules/");
    strcat(alias_opt, kernel_version);
    strcat(alias_opt, "/modules.alias mode=600");
    return add_zip_list_entry(modules_alias_file, ZIP_SRC_OTHER, ZIP_TGT_ROOTFS,
                ENTRY_FILE, parse_entry_options(alias_opt), "") != NULL
                    ? OK : ERR;
}

int
kernmod_init(char const *kernel_version)
{
    int res;
    if ((res = read_modules_dependency_file(kernel_version)) != OK) {
        return res;
    }
    if ((res = read_module_alias_files(kernel_version)) != OK) {
        return res;
    }
    return OK;
}

char **
map_module_name(char const *name)
{
    char **result = NULL;
    int num_entries = 0;
    char *module_name = strsave(name);
    DECLARE_ARRAY_ITER(module_iter, m, struct module_t);

    char *dot = strrchr(module_name, '.');
    if (dot && strcmp(dot, MODULE_SUFFIX) == 0) {
        *dot = '\0';
    }

    result = (char **) malloc(sizeof(char *));
    *result = NULL;

    ARRAY_ITER_LOOP(module_iter, module_array, m) {
        BOOL found = FALSE;

        if (are_modules_equal(m->name, module_name)) {
            found = TRUE;
        }
        else {
            ARRAY_ITER(alias_iter, m->aliases, a, struct alias_t) {
                if (are_modules_equal(a->name, module_name)) {
                    found = TRUE;
                    break;
                }
            }
        }

        if (found) {
            char **newresult;

            ++num_entries;
            newresult = (char **)
                realloc(result, (num_entries + 1) * sizeof(char *));
            if (!newresult) {
                int i;
                for (i = 0; i < num_entries - 1; ++i) {
                    free(result[i]);
                }
                num_entries = 0;
                break;
            }
            else {
                newresult[num_entries - 1] = strsave(m->path);
                newresult[num_entries] = NULL;
                result = newresult;
            }
        }
    }

    free(module_name);

    if (num_entries > 0) {
        int i;
        for (i = 0; i < num_entries; ++i) {
            log_info(LOG_DEP, "#  mapping '%s' to '%s'\n", name, result[i]);
        }
        return result;
    }
    else {
        free(result);
        return NULL;
    }
}

int
resolve_dependencies(char const *kernel_version)
{
    int res;

    log_info (LOG_INFO|LOG_DEP, "resolving open dependencies\n");
    inc_log_indent_level ();

    if ((res = compute_modules_used(kernel_version)) != OK) {
        return res;
    }

    if ((res = add_missing_modules()) != OK) {
        return res;
    }

    if ((res = dump_module_dependency_file(kernel_version)) != OK) {
        return res;
    }

    if ((res = dump_module_alias_file(kernel_version)) != OK) {
        return res;
    }

    dec_log_indent_level ();
    return OK;
}
