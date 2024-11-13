/*****************************************************************************
 *  @file opt.c
 *  Functions for processing opt/package.txt files.
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libglob/src/config.h>
#include <glob.h>

#include <libmkfli4l/defs.h>
#include <libmkfli4l/var.h>
#include <libmkfli4l/regex.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/check.h>
#include <libmkfli4l/options.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/file.h>
#include <elfinfo/readelf.h>
#include "opt.h"
#include "optlibs.h"
#include "kernmod.h"
#include "vartype.h"
#include "var.h"

/**
 * The supported OPT file format version.
 */
#define OPT_FORMAT_VERSION 1

/**
 * The number of archive list entries to allocate in one go.
 */
#define FILES_ARRAY_SIZE 1024
/**
 * The number of uid entries to allocate in one go.
 */
#define UID_ARRAY_SIZE  16
/**
 * The number of gid entries to allocate in one go.
 */
#define GID_ARRAY_SIZE  16
/**
 * The buffer size for detailed regex error messages.
 */
#define SIZE_ERR_BUFFER 256

/**
 * Name of regular expression describing entry options.
 * Its definition can be found in check/base.exp.
 */
#define OPT_REGEXP "OPT_REGEXP"

/**
 * Represents an entry in passwd or group file.
 */
struct id_entry_t {
    /**
     * The name of the user or group.
     */
    char *name;
    /**
     * The associated identifier.
     */
    int id;
};

/**
 * The prefix used to specify that an entry is to be put into the ROOTFS
 * archive.
 */
static const char rootfs_prefix[] = "rootfs:";

char    *packages[MAX_PACKAGES];
int      n_packages = 0;
array_t *files_array;

/**
 * A list of uid entries read from passwd file.
 */
static array_t *uid_array;
/**
 * A list of gid entries read from group file.
 */
static array_t *gid_array;

/**
 * Initializes an entry_t structure.
 *
 * @param entry
 *  The entry structure to initialize.
 */
static void
init_entry(struct entry_t *entry)
{
    entry->entry_type = ENTRY_UNKNOWN;
    entry->filename = NULL;
    entry->prefix = NULL;
    entry->target = NULL;
    entry->zipsrc = ZIP_SRC_UNKNOWN;
    entry->ziptgt = ZIP_TGT_UNKNOWN;
    entry->comment = NULL;
    entry->soname = NULL;
    entry->secondary = FALSE;
    entry->rpath = NULL;
    entry->num_needed = 0;
    entry->needed = NULL;
    entry->mode = 0;
    entry->uid = 0;
    entry->gid = 0;
}

/**
 * Finalizes an entry_t structure.
 *
 * @param entry
 *  The entry structure to finalize.
 */
static void
fini_entry(struct entry_t *entry)
{
    free(entry->filename);
    free(entry->prefix);
    free(entry->target);
    free(entry->comment);
    free(entry->soname);
    free(entry->rpath);
    free(entry->needed);
}

/**
 * Reads a passwd or group file into an array of id_entry_t entries.
 *
 * @param file
 *  The file to read.
 * @param id_array
 *  The array to fill.
 */
static void
read_ids(char const *file, array_t *id_array)
{
    char line[BUF_SIZE];
    FILE *f = fopen(file, "r");
    if (f) {
        while (fgets(line, sizeof line, f) != NULL) {
            char *first_colon = strchr(line, ':');
            char *second_colon = first_colon ? strchr(first_colon + 1, ':') : NULL;
            char *third_colon = second_colon ? strchr(second_colon + 1, ':') : NULL;
            if (second_colon) {
                struct id_entry_t *entry;
                int const len_name = first_colon - line;
                int const len_id = (third_colon ? third_colon - second_colon : (line + strlen(line)) - second_colon) - 1;
                char *name = safe_malloc(len_name + 1);
                char *idstr = safe_malloc(len_id + 1);
                int id;
                strncpy(name, line, len_name);
                name[len_name] = '\0';
                strncpy(idstr, second_colon + 1, len_id);
                idstr[len_id] = '\0';
                if (sscanf(idstr, "%d", &id) == 1) {
                    entry = (struct id_entry_t *)
                        safe_malloc(sizeof(struct id_entry_t));
                    entry->name = name;
                    entry->id = id;
                    append_new_elem(id_array, entry);
                    log_info(LOG_UIDGID, "adding name=%s id=%d from %s\n", name, id, file);
                }
            }
        }
        fclose(f);
    }
}

/**
 * Performs a name-to-(u|g)id mapping.
 *
 * @param name
 *  The name to map.
 * @param id_array
 *  The array to use.
 * @return
 *  A valid (u|g)id number or -1 if the name could not be found.
 */
static int
lookup_id(char const *name, array_t *id_array)
{
    ARRAY_ITER(it, id_array, entry, struct id_entry_t) {
        if (strcmp(name, entry->name) == 0) {
            return entry->id;
        }
    }
    return -1;
}

/**
 * Maps an entry type to a descriptive string.
 *
 * @param entry_type
 *  The entry type to map.
 * @return
 *  The string describing the entry type.
 */
static char const *
map_entry_type_to_string(int entry_type) {
    switch (entry_type) {
    case ENTRY_LOCAL :
        return "local file system object";
    case ENTRY_FILE :
        return "file";
    case ENTRY_DIR :
        return "directory";
    case ENTRY_NODE :
        return "device node";
    case ENTRY_SYMLINK :
        return "symbolic link";
    default :
        return "unknown";
    }
}

/**
 * Checks whether some variable value matches a required value taken from a
 * package file.
 *
 * @param given_value
 *  The given value of a variable, e.g. the value of the variable OPT_PNP.
 * @param required_value
 *  The required value of a variable, e.g. "yes".
 * @param file
 *  The package file containing the required value.
 * @param line
 *  The line in the package file containing the required value.
 * @return
 */
static BOOL
is_match(char const *given_value, char const *required_value, char const *file, int line)
{
    int res;
    BOOL inverse = FALSE;
    char regexp[256];

    if (*required_value == '!') {
        ++required_value;
        inverse = !inverse;
    }
    else if (*required_value == '\\' && *(required_value + 1) == '!') {
        ++required_value;
    }

    strcpy(regexp, "^(");
    strcpy(regexp + 2, required_value);
    strcat(regexp, ")$");
    res = regexp_user(given_value, regexp, 0, 0, 0, file, line);

    log_info (LOG_ZIPLIST_REGEXP, "'%s' %s '%s'\n", regexp,
              res == 0 ? "matches" : "doesn't match",
              given_value);

    return inverse ? res != 0 : res == 0;
}

/**
 * Logs a message when a package file references a variable defined by another
 * package with the exception of the BASE package (BASE variables may always
 * be referenced by any package without causing messages to be logged).
 *
 * @param var
 *  The uppercase name of the referenced variable.
 * @param package
 *  The referencing package.
 */
static void
check_external_opt(char const *var, char const *package)
{
    char const *p = get_variable_package(var);
    if (strcmp(p, "BASE") != 0 &&
            strcmp(p, "_fli4l") != 0 &&
            strcmp(p, package) != 0) {
        log_info (LOG_ZIPLIST, "external reference to '%s' ('%s') in '%s'\n",
                  var, p, package);
    }
}

/**
 * Processes a single entry of a package file.
 *
 * @param var
 *  The name of the enabling variable.
 * @param value
 *  The required value of the enabling variable.
 * @param opt
 *  An array of entry options. May be NULL if no entry options are specified.
 * @param filename
 *  The name of the entry to be added.
 * @param package
 *  The name of the package containing the entry.
 * @param opt_file
 *  The name of the package file containing the entry.
 * @param line
 *  The line in the package file containing the entry.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
check_one_var(char const *var, char const *value, array_t *opt, char const *filename, char const *package, char const *opt_file, int line)
{
    char var_name[BUF_SIZE];
    char var_name2[BUF_SIZE];
    char const *var_contents;
    int error = OK;
    strcpy(var_name, convert_to_upper(var));

    /* check whether it is a *_% variable */
    if (strchr(var_name, '%') == NULL) {
        /* no, it isn't; check whether we have to prefix it with OPT_ */
        var_contents = try_get_variable(var_name);
        if (var_contents == NULL && !check_var_defined(var_name)) {
            strcpy(var_name2, "OPT_");
            strcat(var_name2, var_name);
            var_contents = try_get_variable(var_name2);
            if (var_contents == NULL && !check_var_defined (var_name2)) {
                fatal_exit ("%s:%d: Access to undefined variable %s\n",
                            opt_file, line, var_name);
            }
            strcpy(var_name, var_name2);
        }

        if (is_var_tagged(var_name)) {
            check_external_opt(var_name, package);
            mark_var_requested(var_name);
            if (is_match(var_contents, value, opt_file, line)) {
                if (add_to_zip_list(var_name, var_contents, filename, opt,
                                    opt_file, line) != OK) {
                    error = ERR;
                }
                mark_var_copied(var_name);
            }
            else {
                log_info(LOG_ZIPLIST_SKIP, "- %-40s : %s:'%s' != '%s'\n",
                         filename, var_name, var_contents, value);
            }
        }
        else {
            /* skip file: it's either not a package file which
               should always be copied if OPT_PACKAGE='yes' or
               the variable depends on some other variable
               which isn't set */

            if (!is_var_weak (var_name)) {
                log_info (LOG_ZIPLIST_SKIP, "- %-40s %s "
                          "not active.\n",
                          filename, var_name);
            }
        }
    }
    else {
        char *tmp_var;
        struct iter_t *iter;

        if (!check_var_defined(var_name)) {
            log_error("%s:%d: Undefined variable %s\n", opt_file, line, var_name);
            return ERR;
        }

        iter = init_set_var_iteration(var_name);
        if (iter == NULL) {
            log_error("%s:%d: Unable to get var_n for variable %s\n", opt_file, line, var_name);
            return ERR;
        }

        while ((tmp_var = get_next_set_var(iter)) != NULL) {
            if (try_get_variable(tmp_var) != NULL) {
                if (check_one_var(tmp_var, value, opt, filename, package,
                                  opt_file, line) != OK) {
                    error = ERR;
                }
            }
        }
    }

    return error;
}

/**
 * Processes a single package file.
 *
 * @param file
 *  The package file to process.
 * @param pkg
 *  The package.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
make_package_zip_list (struct package_file_t *file, struct package_t *pkg)
{
    char        buf[BUF_SIZE];
    char        var_name[BUF_SIZE];
    char        value[BUF_SIZE];
    char        filename[BUF_SIZE];
    FILE *      opt_fp;
    char *      p, *opt;
    char *      var;
    int         line;
    int         res = OK;
    int         version;
    int         i;
    struct vartype_t *opt_regex = NULL;
    struct vartype_t *type_none = NULL;
    char const *package = package_get_name(pkg);
    char const *opt_file = package_file_get_name(file);

    opt_fp = fopen(opt_file, "r");
    if (! opt_fp) {
        fatal_exit("Error opening package file '%s': %s\n",
                   opt_file, strerror (errno));
    }

    log_info(LOG_INFO|LOG_ZIPLIST|LOG_ZIPLIST_SKIP, "reading %s\n",  opt_file);
    inc_log_indent_level ();

    line = 0;
    version = 0;
    while (fgets(buf, sizeof(buf), opt_fp)) {
        array_t *parsed_opts = NULL;

        ++line;

        /* strip comments */
        p = strchr(buf, '#');

        if (p) {
            *p = '\0';
        }

        /* skip initial whitespace */
        for (p = buf; *p; ++p) {
            if (*p != ' ' && *p != '\t') {
                break;
            }
        }

        /* skip empty lines */
        if (*p == '\0' || *p == '\r' || *p == '\n') {
            continue;
        }

        if (sscanf(p, "%s %s %s", var_name, value, filename) != 3) {
            fatal_exit ("%s: syntax error: %d '%s'\n",
                        opt_file, *p, p);
        }

        /* search for entry options */
        for (opt = p, i = 0; i < 3; ++i) {
            while (*opt && !isspace((int) *opt++)) {
            }
            while (*opt && isspace((int) *opt)) {
                ++opt;
            }
        }

        if (*opt) {
            p = opt + strlen(opt);
            while ((iscntrl((int) *(p - 1)) || isspace((int) *(p - 1))) && p > opt) {
                --p;
            }
            *p = 0;

            if (*opt) {
                BOOL result;

                if (!opt_regex && !(opt_regex = vartype_get(OPT_REGEXP))) {
                    fatal_exit("unknown regular expression %s, please provide a regular expression describing entry options in check/base.exp\n",
                               OPT_REGEXP);
                }

                result = vartype_match(opt_regex, opt);
                if (!result) {
                    log_error("%s: line %d: unknown entry options '%s'\n", opt_file, line, opt);
                    res = ERR;
                    continue;
                }
                else {
                    parsed_opts = parse_entry_options(opt);
                }
            }
        }

        if (strcmp(var_name, "opt") == 0) {
            /* old style opt/package.txt files */
            fatal_exit("%s: wrong file format (2.0.x format), "
                       "please convert to 2.2.x format\n",
                       package);
        }
        else if (strcmp(var_name, "weak") == 0) {
            /* add weak declaration for variable if it's not already present */
            char *weak_var_name = convert_to_upper(value);
            if (!var_try_get(weak_var_name)) {
                char *weak_var_name2 = strcat_save(strsave("OPT_"), weak_var_name);
                if (!var_try_get(weak_var_name2)) {
                    struct location_t *loc;
                    struct var_properties_t *props;

                    if (!type_none) {
                        type_none = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
                    }

                    loc = location_create(file, line, 0);
                    props = var_properties_create();
                    props->optional = TRUE;
                    if (!var_add(var_get_global_scope(), weak_var_name, type_none, VAR_PRIORITY_CONFIGURATION, loc, props)) {
                        var_properties_destroy(props);
                        location_destroy(loc);
                        res = ERR;
                    }
                }
                free(weak_var_name2);
            }
            continue;
        }
        else if (strcmp(var_name, "opt_format_version") == 0) {
            int v = atoi(value);
            if (v < OPT_FORMAT_VERSION) {
                log_error("%s: incompatible package format found,"
                          " uses version %d, needs version %d\n",
                          package, v, OPT_FORMAT_VERSION);
                res = ERR;
            }
            version = OPT_FORMAT_VERSION;
            continue;
        }

        if (check_opt_files) {
            add_to_zip_list(var_name, "enabled by debug flag",
                            filename, parsed_opts, opt_file, line);
            continue;
        }

        var = var_name;
        while (var) {
            char *next_var = strchr(var, ',');
            if (next_var) {
                *next_var++ = 0;
            }
            if (check_one_var(var, value, parsed_opts, filename, package,
                              opt_file, line) != OK) {
                res = ERR;
            }
            var = next_var;
        }
    }

    if (!version) {
        log_error("%s: Unknown package format found, needs version %d."
                  " If your fli4l router does not work correctly,"
                  " check the dev documentation or ask the author"
                  " of this package.\n",
                  package, OPT_FORMAT_VERSION);
    }

    dec_log_indent_level ();
    fclose (opt_fp);

    return res;
}

/**
 * Loads various data from an ELF file and stores them into an entry_t object.
 *
 * @param p
 *  The entry to process.
 */
static
void load_elf_info(struct entry_t *p)
{
    struct readelf_descriptor *desc;
    if (readelf_process_file(p->filename, &desc) == 0) {
        if (desc->soname) {
            p->soname = strsave(desc->soname);
        } else {
            /* use the target name if no DT_SONAME entry exists */
            char *path = strsave(p->target);
            p->soname = strsave(basename(path));
            free(path);
        }

        if (desc->rpath) {
            p->rpath = strsave(desc->rpath);
        }
        else {
            p->rpath = NULL;
        }

        p->num_needed = desc->num_needed;
        if (p->num_needed == 0) {
            p->needed = NULL;
        } else {
            int i;
            p->needed = (struct elf_dep_t *) safe_malloc(p->num_needed * sizeof(struct elf_dep_t));
            for (i = 0; i < p->num_needed; ++i) {
                p->needed[i].soname = strsave(desc->needed[i]);
                p->needed[i].file   = NULL;
            }
        }

        readelf_free_descriptor(desc);
    }
}

/**
 * Removes a single prefix and following slashes from a file name. If e.g. the
 * file name is "opt/usr/bin/x" and the prefix to remove is "opt",
 * the resulting string is "usr/bin/x".
 *
 * @param name
 *  The file name to process.
 * @param prefix
 *  The prefix to remove.
 * @param len
 *  The length of the prefix to remove.
 * @return
 *  A pointer to the first character behind the prefix. This points directly
 *  to a character in "name", no copy is done.
 */
static char const *
remove_one_prefix(char const *name, char const *prefix, int len)
{
    if (strncmp(name, prefix, len) == 0)
        name += len;
    while (*name == '/')
        name++;
    return name;
}

/**
 * Tries to remove a prefix from a file name in order to determine the name
 * suitable for the target file system. The prefixes to be tried are (1) the
 * configuration directory and (2) "opt".
 *
 * @param name
 *  The file name to process.
 * @return
 *  A pointer to the first character behind the prefix (whatever it is). This
 *  points directly to a character in "name", no copy is done.
 */
static char const *
remove_prefix(char const *name)
{
    char opt_prefix[] = "opt";
    name = remove_one_prefix (name, config_dir_with_slashes, config_dir_len);
    name = remove_one_prefix (name, opt_prefix, sizeof(opt_prefix)-1);
    return name;
}

/**
 * Returns the prefix of the file name that is removed for the target file
 * system on the router. The following equation holds:
 * s == get_prefix(s) + remove_prefix(s)
 * where "+" is the string concatenation.
 *
 * Example: opt/etc/issue --> opt/
 *
 * @param filename
 *  The file name to process.
 * @return
 *  A dynamically allocated string containing the prefix.
 */
static char *
get_prefix(char const *filename)
{
    char const *p = remove_prefix(filename);
    int const len = p - filename;
    char *result = (char *) safe_malloc(len + 1);
    strncpy(result, filename, len);
    result[len] = '\0';
    return result;
}

/**
 * Searches for a certain entry option.
 *
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @param name
 *  The name of the entry option to be searched for.
 * @return
 *  The value of the option or NULL if option could not be found. The string
 *  returned is not copied; if it is to be stored somewhere, the caller must
 *  use strsave() to copy it.
 */
static char *
lookup_entry_option(array_t *opts, char const *name)
{
    if (opts) {
        ARRAY_ITER(it, opts, opt, struct opt_t) {
            if (strcmp(name, opt->name) == 0) {
                return opt->value;
            }
        }
    }

    return NULL;
}

/**
 * Reads and converts "flags" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @return
 *  The entry conversion type. If no conversion is specified, CONV_NONE is
 *  returned. If an invalid conversion is specified, CONV_UNKNOWN is returned.
 */
static int
read_entry_conv_type(char const *filename, char const *target, array_t *opts)
{
    char *value = lookup_entry_option(opts, "flags");
    if (value) {
        if (strcmp(value, "utxt") == 0) {
            return CONV_UTXT;
        }
        if (strcmp(value, "dtxt") == 0) {
            return CONV_DTXT;
        }
        if (strcmp(value, "sh") == 0) {
            return CONV_SH;
        }
        if (strcmp(value, "luac") == 0) {
            return CONV_LUAC;
        }
        log_error("%s: invalid conversion type '%s' specified for target '%s'\n",
                  filename, value, target);
        return CONV_UNKNOWN;
    }
    return CONV_NONE;
}

/**
 * Reads and converts "mode" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @param entry_type
 *  The entry type. If no "mode" option is available, a default mode is
 *  returned based on this parameter. If an invalid "mode" option is specified,
 *  -1 is returned.
 * @return
 *  The entry mode.
 */
static int
read_entry_mode(char const *filename, char const *target, array_t *opts, int entry_type)
{
    char *value = lookup_entry_option(opts, "mode");
    if (value) {
        int mode;
        if (sscanf(value, "%o", (unsigned *) &mode) != 1 || mode < 0) {
            log_error("%s: invalid mode '%s' specified for target '%s'\n",
                      filename, value, target);
            return -1;
        }

        return mode;
    } else {
        switch (entry_type) {
        case ENTRY_SYMLINK :
            return 0777;
        case ENTRY_DIR :
            return 0755;
        default :
            return 0644;
        }
    }
}

/**
 * Reads and converts "uid" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @return
 *  The uid number. If no "uid" option could be found, zero is returned. If an
 *  unknown user name is speficied, -1 is returned.
 */
static int
read_entry_uid(char const *filename, char const *target, array_t *opts)
{
    char *value = lookup_entry_option(opts, "uid");
    if (value) {
        int uid;
        if (sscanf(value, "%d", &uid) != 1) {
            uid = lookup_id(value, uid_array);
            if (uid < 0) {
                log_error("%s: unknown user name '%s' specified for target '%s'\n",
                          filename, value, target);
                return -1;
            }
        }
        else if (uid < 0) {
            log_error("%s: invalid uid '%s' specified for target '%s'\n",
                      filename, value, target);
            return -1;
        }

        return uid;
    } else {
        return 0;
    }
}

/**
 * Reads and converts "gid" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @return
 *  The gid number. If no "gid" option could be found, zero is returned. If an
 *  unknown group name is speficied, -1 is returned.
 */
static int
read_entry_gid(char const *filename, char const *target, array_t *opts)
{
    char *value = lookup_entry_option(opts, "gid");
    if (value) {
        int gid;
        if (sscanf(value, "%d", &gid) != 1) {
            gid = lookup_id(value, gid_array);
            if (gid < 0) {
                log_error("%s: unknown group name '%s' specified for target '%s'\n",
                          filename, value, target);
                return -1;
            }
        }
        else if (gid < 0) {
            log_error("%s: invalid gid '%s' specified for target '%s'\n",
                      filename, value, target);
            return -1;
        }

        return gid;
    } else {
        return 0;
    }
}

/**
 * Reads and converts "devtype" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @return
 *  The device type or -1 if no "devtype" option could be found.
 */
static char
read_entry_devtype(char const *filename, char const *target, array_t *opts)
{
    char *value = lookup_entry_option(opts, "devtype");
    if (value) {
        char devtype;
        if (sscanf(value, "%c", &devtype) != 1 || (devtype != 'c' && devtype != 'b')) {
            log_error("%s: invalid device type '%s' specified for target '%s'\n",
                      filename, value, target);
            return -1;
        }

        return devtype;
    }
    else {
        log_error("%s: missing device type for target '%s'\n",
                  filename, target);
        return -1;
    }
}

/**
 * Reads and converts "major" or "minor" option.
 *
 * @param filename
 *  The name of the file.
 * @param target
 *  The name of the target entry.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @param name
 *  The name of the option to process. Must be "major" or "minor".
 * @return
 *  The device number or -1 if option could no be found or is invalid.
 */
static int
read_entry_devnum(char const *filename, char const *target, array_t *opts, char const *name)
{
    char *value = lookup_entry_option(opts, name);
    if (value) {
        int number;
        if (sscanf(value, "%d", &number) != 1 || number < 0) {
            log_error("%s: invalid device %s '%s' specified for target '%s'\n",
                      filename, name, value, target);
            return -1;
        }
        else {
            return number;
        }
    } else {
        log_error("%s: missing device %s for target '%s'\n",
                  filename, name, target);
        return -1;
    }
}

/**
 * Reads and converts "linktarget" option.
 *
 * @param filename
 *  The name of the file.
 * @param opts
 *  The option array as returned by parse_entry_options(). May be NULL.
 * @return
 *  The link target or the empty string if option could no be found.
 */
static char *
read_entry_linktarget(char const *filename, array_t *opts)
{
    char *result = lookup_entry_option(opts, "linktarget");
    return strsave(result ? result : "");
    (void) filename;
}

/**
 * Searches for a file to be added given a prefix.
 *
 * @param pref
 *  The prefix to try (opt, config etc.).
 * @param filename
 *  The file to search for without a prefix (opt, config etc.).
 * @param glob_res
 *  Receives the result of globbing.
 * @param flags
 *  The globbing flags. See documentation of glob() for details.
 * @return
 *  TRUE if at least one file has been found, FALSE otherwise.
 */
static int
exec_glob(char const *pref, char const *filename, glob_t *glob_res, int flags)
{
    char buf[BUF_SIZE];
    sprintf(buf, "%s%s%s", pref ? pref : "", pref ? "/" : "", filename);
    return glob(buf, flags, NULL, glob_res);
}

/**
 * Returns a file's entry type. This will never be ENTRY_LOCAL but may be
 * ENTRY_UNKNOWN.
 *
 * @param filename
 *  The file to examine.
 * @return
 *  The entry type of the file.
 */
static int
get_entry_type(char const *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return ENTRY_FILE;
        }
        else if (S_ISDIR(st.st_mode)) {
            return ENTRY_DIR;
        }
        else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
            return ENTRY_NODE;
        }
#if defined(S_ISLNK)
        else if (S_ISLNK(st.st_mode)) {
            return ENTRY_SYMLINK;
        }
#endif
        else {
            return ENTRY_UNKNOWN;
        }
    }
    else {
        return ENTRY_UNKNOWN;
    }
}

/**
 * Searches for a file to be added.
 *
 * @param filename
 *  The file to search for without a prefix (opt, config etc.).
 * @param zip_src
 *  Receives the source of the file if found (some ZIP_SRC_* constant).
 * @param glob_res
 *  Receives the result of globbing.
 * @param flags
 *  The globbing flags. See documentation of glob() for details.
 * @return
 *  OK on success, ERR otherwise.
 */
static int
do_glob(char const *filename, int *zip_src, glob_t *glob_res, int flags)
{
    if (exec_glob (config_dir_with_slashes, filename, glob_res, flags) == 0)
    {
        *zip_src = ZIP_SRC_CONF;
        return OK;
    }

    if (exec_glob ("opt", filename, glob_res, flags) == 0)
    {
        *zip_src = ZIP_SRC_OPT;
        return OK;
    }

    /* necessary for accessing img/modules.dep and img/modules.alias */
    if (exec_glob (NULL, filename, glob_res, flags) == 0)
    {
        *zip_src = ZIP_SRC_OTHER;
        return OK;
    }

    return ERR;
}

/**
 * Compares two entry_t objects by target name.
 *
 * @param entry1
 *  The first entry.
 * @param entry2
 *  The second entry.
 * @return
 *  Zero if the target names are equal, a value less than zero if the first
 *  target name is less than the second one, otherwise a value greater than
 *  zero.
 */
static int
entry_target_compare(void const *entry1, void const *entry2)
{
    return strcmp(((struct entry_t *) (*(void **) entry1))->target,
                  ((struct entry_t *) (*(void **) entry2))->target);
}

/**
 * Adds entries for directories containing other entries.
 *
 * @return
 *  OK on success, ERR otherwise.
 */
static int
add_directories(void)
{
    ARRAY_ITER(array_iter, files_array, first, struct entry_t) {
        char *dir = strsave(first->target);
        while (TRUE) {
            char *slash  = strrchr(dir, '/');
            if (slash) {
                *slash = '\0';
                if (add_zip_list_entry(dir, ZIP_SRC_OTHER, first->ziptgt,
                        ENTRY_DIR, NULL, "") == NULL) {
                    free(dir);
                    return ERR;
                }
            }
            else
                break;
        }
        free(dir);
    }

    return OK;
}

struct entry_t *
get_zip_list_target_entry(char const *target)
{
    ARRAY_ITER(array_iter, files_array, p, struct entry_t)
    {
        if (p->target && *p->target && ! strcmp (p->target, target))
        {
            return p;
        }
    }
    return 0;
}

static BOOL zip_handle_package(void *entry, void *data)
{
    struct package_t *package = (struct package_t *) entry;
    struct package_file_t *file = package_get_file(package, PACKAGE_FILE_TYPE_IMAGE);

    if (!file) {
        return TRUE;
    }
    else {
        return make_package_zip_list(file, package) == OK;
    }

    UNUSED(data);
}

int
make_zip_list (char const * kernel_version)
{
    char        kernel_major[]   = "KERNEL_MAJOR";
    int         res = OK;
    struct vartype_t *type_kernel_major;
    struct var_t *var_kernel_major;
    struct var_instance_t *varinst_kernel_major;

    /* use <x>.<y> as major version */
    char *kernel_major_version = strsave(kernel_version);
    char *dot = strchr(kernel_major_version, '.');
    if (dot) {
        ++dot;
        if ((dot = strchr(dot, '.')) != NULL) {
            *dot = '\0';
        }
    }

    type_kernel_major = vartype_get(kernel_major);
    if (!type_kernel_major) {
        return ERR;
    }
    var_kernel_major = var_add(
        var_get_global_scope(),
        kernel_major,
        type_kernel_major,
        VAR_PRIORITY_CONFIGURATION,
        location_create_internal(PACKAGE_FILE_TYPE_VARDEF),
        var_properties_create()
    );
    if (!var_kernel_major) {
        return ERR;
    }

    varinst_kernel_major = var_instance_create(var_kernel_major, 0, NULL);
    if (!var_instance_set_and_check_value(varinst_kernel_major, kernel_major_version,
            NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION)) {
        return ERR;
    }

    free(kernel_major_version);

    if ((res = kernmod_init(kernel_version)) != OK) {
        return res;
    }

    return package_foreach(zip_handle_package, NULL) ? OK : ERR;
}

void zip_list_init (void)
{
    files_array = init_array (FILES_ARRAY_SIZE);
    uid_array = init_array (UID_ARRAY_SIZE);
    gid_array = init_array (GID_ARRAY_SIZE);
    inc_log_indent_level ();
    read_ids("opt/etc/passwd", uid_array);
    read_ids("opt/etc/group", gid_array);
    dec_log_indent_level ();
}

struct entry_t *
add_zip_list_entry(char const *file, int zipsrc, int ziptgt, int entry_type, array_t *opts, char const *comment)
{
    struct entry_t *p;
    struct entry_t tmp_entry;
    char *target;

    if (entry_type == ENTRY_UNKNOWN || entry_type == ENTRY_LOCAL) {
        fatal_exit("Internal error in add_zip_list_entry(): entry type '%s' not allowed!\n", map_entry_type_to_string(entry_type));
    }

    target = lookup_entry_option(opts, "name");
    if (!target) {
        target = strsave(remove_prefix(file));
    }
    else {
        target = strsave(target);
    }

    if (strchr(target, ' ') != NULL) {
        log_error("Invalid character in filename '%s' added via '%s' - spaces are not allowed\n",
                  target, file);
        free(target);
        return NULL;
    }

    p = get_zip_list_target_entry(target);
    if (!p) {
        init_entry(&tmp_entry);

        tmp_entry.entry_type = entry_type;
        tmp_entry.filename = strsave(file);
        tmp_entry.zipsrc = zipsrc;
        tmp_entry.ziptgt = ziptgt;
        tmp_entry.prefix = get_prefix(tmp_entry.filename);
        tmp_entry.comment = comment ? strsave(comment) : strsave("");
        tmp_entry.target = target;

        if (strchr(tmp_entry.target, ' ')) {
            log_error("Invalid character in filename '%s' added via '%s' - spaces are not allowed\n",
                      tmp_entry.target, file);
            fini_entry(&tmp_entry);
            return NULL;
        }

        tmp_entry.mode = read_entry_mode(file, tmp_entry.target, opts, entry_type);
        if (tmp_entry.mode < 0) {
            fini_entry(&tmp_entry);
            return NULL;
        }

        tmp_entry.uid = read_entry_uid(file, tmp_entry.target, opts);
        if (tmp_entry.uid < 0) {
            fini_entry(&tmp_entry);
            return NULL;
        }

        tmp_entry.gid = read_entry_gid(file, tmp_entry.target, opts);
        if (tmp_entry.gid < 0) {
            fini_entry(&tmp_entry);
            return NULL;
        }

        load_elf_info(&tmp_entry);

        switch (entry_type) {
        case ENTRY_FILE :
            tmp_entry.u.file.conv_type = read_entry_conv_type(file, tmp_entry.target, opts);
            if (tmp_entry.u.file.conv_type == CONV_UNKNOWN) {
                fini_entry(&tmp_entry);
                return NULL;
            }
            break;

        case ENTRY_NODE :
            tmp_entry.u.node.devtype = read_entry_devtype(file, tmp_entry.target, opts);
            if (tmp_entry.u.node.devtype < 0) {
                fini_entry(&tmp_entry);
                return NULL;
            }

            tmp_entry.u.node.major = read_entry_devnum(file, tmp_entry.target, opts, "major");
            if (tmp_entry.u.node.major < 0) {
                fini_entry(&tmp_entry);
                return NULL;
            }

            tmp_entry.u.node.minor = read_entry_devnum(file, tmp_entry.target, opts, "minor");
            if (tmp_entry.u.node.minor < 0) {
                fini_entry(&tmp_entry);
                return NULL;
            }

            break;

        case ENTRY_SYMLINK :
            tmp_entry.u.symlink.linktarget = read_entry_linktarget(file, opts);
            break;
        }

        p = (struct entry_t *) safe_malloc(sizeof(struct entry_t));
        *p = tmp_entry;
        append_new_elem(files_array, p);

        log_info(LOG_ZIPLIST, "+  %s: %s %-40s --> %-40s %s\n",
                 get_location(ziptgt), map_entry_type_to_string(entry_type),
                 file, p->target, comment);
    }
    else {
        if (entry_type != p->entry_type) {
            log_error("'%s': Cannot change entry type from '%s' to '%s'\n",
                      target, map_entry_type_to_string(p->entry_type),
                      map_entry_type_to_string(entry_type));
            free(target);
            return NULL;
        }
        else if (strcmp(file, p->filename) != 0 && entry_type != ENTRY_DIR) {
            log_error("'%s' has multiple sources: %s '%s' (due to '%s') and %s '%s' (due to '%s')\n",
                      target,
                      map_entry_type_to_string(p->entry_type),
                      p->filename,
                      p->comment,
                      map_entry_type_to_string(entry_type),
                      file,
                      comment ? comment : "");
            free(target);
            return NULL;
        }
        else if (ziptgt < p->ziptgt) {
            char const *oldtype = get_location(p->ziptgt);
            p->ziptgt = ziptgt;
            log_info(LOG_ZIPLIST, "%s -> %s: %-40s %s\n",
                     oldtype, get_location(ziptgt), target, comment);
        }
        free(target);
    }

    return p;
}

static int
add_entry_to_zip_list(char const *var, char const *content, char const *filename,
    char const *found, array_t *opt, int zipsrc, int ziptgt, int entry_type,
    BOOL must_exist, char const *opt_file, int line)
{
    char tmp[BUF_SIZE];
    int real_entry_type;

    if (zipsrc == ZIP_SRC_CONF) {
        log_info(LOG_INFO, "#  using %s/%s instead of normal version\n",
                 config_dir, filename);
    }

    real_entry_type = get_entry_type(found);

    if (entry_type == ENTRY_LOCAL) {
        entry_type = real_entry_type;
    }
    else if (entry_type != real_entry_type &&
                (must_exist || real_entry_type != ENTRY_UNKNOWN)) {
        log_error("%s:%d: entry '%s' is of type '%s' but is specified to be of type '%s'\n",
                  opt_file, line, filename,
                  map_entry_type_to_string(real_entry_type),
                  map_entry_type_to_string(entry_type));
        return ERR;
    }

    if (entry_type == ENTRY_UNKNOWN) {
        log_error("%s:%d: entry '%s' is of unknown type\n", opt_file, line, filename);
        return ERR;
    }

    if (!is_var_tagged(var) && !check_opt_files) {
        log_error("(%s='%s') not flagged, but file copied\n", var, content);
        return ERR;
    }

    snprintf(tmp, sizeof(tmp) - 1, "(%s='%s' %s:%d)",
             var, content, opt_file, line);
    return add_zip_list_entry(found, zipsrc, ziptgt, entry_type, opt, tmp) != NULL
        ? OK : ERR;
}

int
add_to_zip_list(char const *var, char const *content, char const *filename, array_t *opt, char const *opt_file, int line)
{
    char        tmp[BUF_SIZE];
    int         zipsrc = ZIP_SRC_UNKNOWN;
    int         ziptgt = ZIP_TGT_UNKNOWN;
    int         res;
    glob_t      glob_res;
    int         entry_type = ENTRY_LOCAL;
    BOOL        must_exist = TRUE;
    char const *type_opt;
    char const *found = NULL;
    size_t      i;

    if (!strncmp(filename, rootfs_prefix, sizeof(rootfs_prefix) - 1)) {
        filename += sizeof(rootfs_prefix) - 1;
        ziptgt = ZIP_TGT_ROOTFS;
    }
    else {
        ziptgt = ZIP_TGT_OPT;
    }

    filename = parse_rewrite_string(filename, opt_file, line);

    type_opt = lookup_entry_option(opt, "type");
    if (type_opt) {
        if (strcmp(type_opt, "file") == 0) {
            entry_type = ENTRY_FILE;
        }
        else if (strcmp(type_opt, "dir") == 0) {
            entry_type = ENTRY_DIR;
            must_exist = FALSE;
        }
        else if (strcmp(type_opt, "node") == 0) {
            entry_type = ENTRY_NODE;
            must_exist = FALSE;
        }
        else if (strcmp(type_opt, "symlink") == 0) {
            entry_type = ENTRY_SYMLINK;
            must_exist = FALSE;
        }
        else if (strcmp(type_opt, "local") != 0) {
            log_error("%s:%d: unknown entry type '%s'\n", opt_file, line, type_opt);
            return ERR;
        }
    }

    if ((res = do_glob(filename, &zipsrc, &glob_res, 0)) != OK) {
        char **module_paths = map_module_name(filename);
        if (module_paths) {
            char **m = module_paths;
            while (*m) {
                int tmpres = ERR;
                sprintf(tmp, ".%s", *m);
                tmpres = do_glob(tmp, &zipsrc, &glob_res,
                                m == module_paths ? 0 : GLOB_APPEND);
                if (tmpres == OK) {
                    res = OK;
                }
                free(*m);
                ++m;
            }
            free(module_paths);
        }
    }

    if (res != OK) {
        if (must_exist) {
            log_error("%s:%d: cannot access '%s' of type '%s'\n", opt_file, line, filename, map_entry_type_to_string(entry_type ));
        }
        else {
            res = add_entry_to_zip_list(var, content, filename, filename, opt,
                                            ZIP_SRC_OTHER, ziptgt, entry_type,
                                            FALSE, opt_file, line);
        }
    }
    else {
        for (i = 0; i < glob_res.gl_pathc; ++i) {
            found = glob_res.gl_pathv[i];

            if ((res = add_entry_to_zip_list(var, content, filename, found, opt,
                                                zipsrc, ziptgt, entry_type,
                                                TRUE, opt_file, line)) != OK) {
                break;
            }
        }
    }

    globfree(&glob_res);
    return res;
}

char const *
get_location(int tgt)
{
    switch (tgt) {
    case ZIP_TGT_ROOTFS :
        return "rootfs";
    case ZIP_TGT_OPT :
        return "opt";
    default :
        return "unknown";
    }
}

char const *
get_conversion_type(int conv_type)
{
    switch (conv_type) {
    case CONV_NONE :
        return "none";
    case CONV_UTXT :
        return "utxt";
    case CONV_DTXT :
        return "dtxt";
    case CONV_SH :
        return "sh";
    case CONV_LUAC :
        return "luac";
    case CONV_UNKNOWN :
    default:
        return "unknown";
    }
}

int
prepare_zip_lists (void)
{
    void **beg = get_array_start(files_array);
    void **end = get_array_end(files_array);
    int res;

    qsort (beg, end - beg, sizeof (*beg), entry_target_compare);

    log_info (LOG_INFO|LOG_ZIPLIST, "adding missing directories\n");
    inc_log_indent_level ();
    if ((res = add_directories()) < 0) {
        return res;
    }
    dec_log_indent_level ();

    beg = get_array_start(files_array);
    end = get_array_end(files_array);
    qsort (beg, end - beg, sizeof (*beg), entry_target_compare);

    return OK;
}
