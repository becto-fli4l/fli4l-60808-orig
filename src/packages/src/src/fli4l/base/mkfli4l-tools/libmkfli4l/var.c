/*----------------------------------------------------------------------------
 *  var.c - functions for processing variables
 *
 *  Copyright (c) 2001 - Frank Meyer
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
 *  Creation:    2001-08-12  fm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#if !defined(PLATFORM_mingw32)
#include <sys/utsname.h>
#endif

#include "defs.h"
#include "var.h"
#include "regex.h"
#include "check.h"
#include "log.h"
#include "str.h"
#include "cfg.h"
#include "array.h"
#include "options.h"
#include "grammar/varass.h"
#include "grammar/varass_parser.h"
#include "grammar/varass_scanner.h"
#include "package.h"
#include "vartype.h"
#include "container/hashmap.h"
#include "container/hashfunc.h"
#include "container/cmpfunc.h"
#include "../var.h"

var_flags_t *create_var_flags(BOOL generated, char const *set_var)
{
    var_flags_t *result = (var_flags_t *) safe_malloc(sizeof(var_flags_t));
    result->generated = generated;
    result->set_var = strsave(set_var);
    return result;
}

void *clone_var_flags(void *data)
{
    var_flags_t *oldflags = (var_flags_t *) data;
    var_flags_t *newflags = (var_flags_t *) safe_malloc(sizeof(var_flags_t));
    newflags->generated = oldflags->generated;
    newflags->set_var = strsave(oldflags->set_var);
    return newflags;
}

void free_var_flags(void *data)
{
    var_flags_t *flags = (var_flags_t *) data;
    free(flags->set_var);
    free(flags);
}

static var_flags_t *get_var_flags(struct var_properties_t const *props)
{
    return props->extra ? (var_flags_t *) props->extra->data : NULL;
}

slot_flags_t *create_slot_flags(void)
{
    slot_flags_t *result = (slot_flags_t *) safe_malloc(sizeof(slot_flags_t));
    result->requested = FALSE;
    result->copied = FALSE;
    return result;
}

static struct varass_parse_tree_t *tree;

/**
 * Stores a pointer to a varass_index_t object together with the associated
 * absolute index (if any).
 */
struct varass_eval_index_t {
    /**
     * The original index from the parser.
     */
    struct varass_index_t *index;
    /**
     * TRUE if index has been resolved to an absolute index, else FALSE.
     */
    BOOL resolved;
    /**
     * The absolute index. Only meaningful if "resolved" is TRUE.
     */
    unsigned abs_index;
};

/**
 * Describes a variable within a package.
 */
struct var_entry_t {
    /**
     * The variable name.
     */
    char const *name;
    /**
     * The variable itself.
     */
    struct var_t *var;
};

/**
 * Describes a package.
 */
struct package_entry_t {
    /**
     * The package name.
     */
    char const *name;
    /**
     * An array of variables (var_entry_t objects) belonging to this package.
     */
    array_t *var_array;
};

/**
 * Compares two var_entry_t objects in an array by looking at the variable
 * names.
 *
 * @param lhs
 *  The left entry.
 * @param rhs
 *  The right entry.
 * @retval 0 if both entries are equal.
 * @retval -1 if the left entry is less than the right one.
 * @retval 1 if the right entry is less than the left one.
 */
static int
my_qsort_cmp_func(void const *lhs, void const *rhs)
{
    struct var_entry_t const *l = *(struct var_entry_t const **) lhs;
    struct var_entry_t const *r = *(struct var_entry_t const **) rhs;
    return strcasecmp(l->name, r->name);
}

/**
 * Destroys a package_entry_t object.
 *
 * @param entry
 *  The object to be freed.
 */
static void
destroy_package_entry(void *entry)
{
    struct package_entry_t *pkg_entry = (struct package_entry_t *) entry;
    free_array(pkg_entry->var_array);
    free(pkg_entry);
}

/**
 * Assigns a variable to "its" package.
 *
 * @param entry
 *  A variable.
 * @param data
 *  A hash map containing package_entry_t objects.
 * @return
 *  Always TRUE.
 */
static BOOL
collect_variable(void *entry, void *data)
{
    struct var_entry_t *var_entry;
    struct package_file_t *file;
    char const *pkg_name;
    struct package_entry_t *pkg_entry;

    struct var_t *var = (struct var_t *) entry;
    struct hashmap_t *map = (struct hashmap_t *) data;

    if (var_get_priority(var) < VAR_PRIORITY_CONFIGURATION) {
        return TRUE;
    }

    file = location_get_package_file(var_get_location(var));
    pkg_name = package_get_name(package_file_get_package(file));
    pkg_entry = (struct package_entry_t *) hashmap_get(map, &pkg_name);
    if (!pkg_entry) {
        pkg_entry = (struct package_entry_t *)
            safe_malloc(sizeof(struct package_entry_t));
        pkg_entry->name = pkg_name;
        pkg_entry->var_array = init_array(64);
        hashmap_put(map, pkg_entry);
    }

    var_entry = (struct var_entry_t *) safe_malloc(sizeof(struct var_entry_t));
    var_entry->name = var_get_name(var);
    var_entry->var = var;
    append_new_elem(pkg_entry->var_array, var_entry);

    return TRUE;
}

/**
 * Contains data related to dumping.
 */
struct dump_data_t {
    /**
     * The file to dump to.
     */
    FILE *fp;
    /**
     * The number of variables dumped.
     */
    int num_total;
    /**
     * TRUE if the dump was successful, FALSE otherwise.
     */
    BOOL result;
};

/**
 * Dumps a value. Handles masking of characters and the selection of the
 * correct delimiters.
 *
 * @return
 *  TRUE if the dump was successful, FALSE otherwise.
 */
static BOOL
dump_value(FILE *fp, char const *value)
{
    if (strchr(value, '\'') == NULL) {
        return fprintf(fp, "'%s'", value) >= 0;
    }
    else {
        BOOL result;
        char *s = (char *) safe_malloc(
            strlen(value) * 2 /* each character could be masked */
            + 1 /* the NUL byte */
        );

        char *p = s;
        while (*value) {
            if (*value == '"' || *value == '\\') {
                *p++ = '\\';
                *p++ = *value;
            }
            else {
                *p++ = *value;
            }
            ++value;
        }
        *p = '\0';

        result = fprintf(fp, "\"%s\"", s) >= 0;
        free(s);
        return result;
    }
}

/**
 * Dumps a complete variable. If an array variable is passed, all values of
 * the array are dumped.
 *
 * @param entry
 *  A var_entry_t object describing the variable to be dumped.
 * @param data
 *  A dump_data_t object.
 * @return
 *  TRUE if the dump was successful, FALSE otherwise.
 */
static BOOL
dump_variable(void *entry, void *data)
{
    struct var_entry_t *var_entry = (struct var_entry_t *) entry;
    struct dump_data_t *dump_data = (struct dump_data_t *) data;

    struct var_instance_t *inst;
    struct var_value_iterator_t *iter;

    inst = var_instance_create(var_entry->var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);

    while (var_value_iterator_next(iter)) {
        struct var_value_t *value = var_value_iterator_get(iter);
        if (value->value && value->kind != VAR_VALUE_KIND_DISABLED) {
            char *name = var_instance_to_string(var_value_iterator_get_var_instance(iter));
            dump_data->result = fprintf(dump_data->fp, "%s=", name)
                    && dump_value(dump_data->fp, value->value)
                    && fprintf(dump_data->fp, "\n")
                    && dump_data->result;
            ++dump_data->num_total;
            free(name);
        }
    }

    var_value_iterator_destroy(iter);
    var_instance_destroy(inst);
    return TRUE;
}

/**
 * Dumps a complete package.
 *
 * @param entry
 *  A package_entry_t object describing the package to be dumped.
 * @param data
 *  A dump_data_t object.
 * @return
 *  TRUE if the dump was successful, FALSE otherwise.
 */
static BOOL
dump_package(void *entry, void *data)
{
    struct package_entry_t *pkg_entry = (struct package_entry_t *) entry;
    struct dump_data_t *dump_data = (struct dump_data_t *) data;
    DECLARE_ARRAY_ITER(iter, var, struct var_t);
    void **beg;
    void **end;

    dump_data->result = fprintf(dump_data->fp, "\n#\n# package '%s'\n#\n",
                                    pkg_entry->name) >= 0 && dump_data->result;

    beg = get_array_start(pkg_entry->var_array);
    end = get_array_end(pkg_entry->var_array);
    qsort(beg, end - beg, sizeof(*beg), my_qsort_cmp_func);

    ARRAY_ITER_LOOP(iter, pkg_entry->var_array, var) {
        dump_variable(var, dump_data);
    }

    return TRUE;
}

BOOL
dump_variables(char const *fname)
{
    FILE *fp;
    struct dump_data_t *dump_data;
    BOOL ret = TRUE;

#if !defined(PLATFORM_mingw32)
    struct utsname sys;
#endif

    log_info(LOG_INFO, "generating %s\n", fname);

    fp = fopen(fname, "wb");
    if (!fp) {
        log_error("Error opening '%s': %s\n", fname, strerror (errno));
        return FALSE;
    }

#if !defined(PLATFORM_mingw32)
    if (uname(&sys) != -1) {
        if (fprintf(fp, "#\n# generated by mkfli4l (%s) running under "
                    "%s Version %s\n#\n", BUILD_DATE, sys.sysname, sys.release) < 0) {
            log_error("Error while writing to %s: %s\n", fname, strerror(errno));
            ret = FALSE;
        }
    }
    else
#endif
    if (fprintf(fp, "#\n# generated by mkfli4l (%s)\n#\n", BUILD_DATE) < 0) {
        log_error("Error while writing to %s: %s\n", fname, strerror(errno));
        ret = FALSE;
    }

    if (ret) {
        struct hashmap_t *map = hashmap_create(
            offsetof(struct package_entry_t, name),
            16,
            &hash_ci_string,
            &compare_ci_string_keys,
            &destroy_package_entry
        );

        var_scope_foreach(var_get_global_scope(), FALSE, collect_variable, map);

        dump_data = (struct dump_data_t *)
            safe_malloc(sizeof(struct dump_data_t));
        dump_data->fp = fp;
        dump_data->num_total = 0;
        dump_data->result = TRUE;
        hashmap_foreach(map, dump_package, dump_data);
        ret = dump_data->result;
        free(dump_data);

        hashmap_destroy(map);

        if (ret) {
            inc_log_indent_level();
            log_info(LOG_INFO, "total number of variables : %d\n", dump_data->num_total);
            log_info(LOG_INFO, "total size of variables   : %ld\n", ftell(fp));
            dec_log_indent_level();
        }
    }

    fclose(fp);

    if (!ret) {
        unlink(fname);
    }

    return ret;
}

char const *
get_variable(char const *name)
{
    return var_read(name);
}

char const *
try_get_variable(char const *name)
{
    return var_try_read(name);
}

char const *
get_variable_package(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), name);
    if (inst) {
        var_flags_t *flags = get_var_flags(var_get_properties(inst->var));
        if (flags && flags->set_var) {
            return flags->set_var;
        }
        else {
            struct location_t const *loc = var_get_location(inst->var);
            var_instance_destroy(inst);
            if (loc) {
                struct package_file_t *file = location_get_package_file(loc);
                if (file) {
                    struct package_t *package = package_file_get_package(file);
                    if (package) {
                        return package_get_name(package);
                    }
                }
            }
        }
    }

    return NULL;
}

char const *
get_variable_comment(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), name);
    if (inst) {
        struct var_properties_t const *props = var_get_properties(inst->var);
        var_instance_destroy(inst);
        return props->comment;
    }
    else {
        return NULL;
    }
}

char *
get_variable_src(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), name);
    if (inst) {
        struct location_t const *loc = var_get_location(inst->var);
        var_instance_destroy(inst);
        if (loc) {
            return location_toString(loc);
        }
    }

    return strsave("(unknown)");
}

static slot_flags_t *
get_variable_slot_extra(struct var_instance_t *inst)
{
    struct var_value_t *value = var_instance_try_get_value(inst);
    if (value) {
        struct var_slot_extra_t *extra = value->extra;
        if (extra) {
            slot_flags_t *const result = (slot_flags_t *) value->extra->data;
            var_value_destroy(value);
            return result;
        } else {
            slot_flags_t *flags = create_slot_flags();
            extra = var_slot_extra_create(flags, free);
            var_value_destroy(value);
            if (!var_instance_set_value(inst, NULL, extra, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_PERSISTENT)) {
                var_slot_extra_destroy(extra);
                return NULL;
            } else {
                return flags;
            }
        }
    } else {
        return NULL;
    }
}

void
var_init(void)
{
    tree = varass_parse_tree_create();
}

void
read_config_file(struct package_file_t *file, struct package_t *package, int priority)
{
    char const *config_file = package_file_get_name(file);
    FILE *f = fopen(config_file, "r");
    if (f) {
        yyscan_t scanner;
        int parse_result;

        if (varass_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", config_file);
            fclose(f);
            tree->ok = FALSE;
        }
        else {
            varass_set_in(f, scanner);
            parse_result = varass_parse(scanner, file, tree, priority);
            varass_lex_destroy(scanner);
            fclose(f);

            if (parse_result != 0) {
                tree->ok = FALSE;
            }
        }
    }
    else {
        log_error("Error opening configuration file %s\n", config_file);
        tree->ok = FALSE;
    }

    (void) package;
}

static void
config_process_target_part(
    struct varass_target_t *target,
    char **var_name,
    int *num_indices,
    struct varass_eval_index_t **indices
)
{
    switch (target->kind) {
    case VARASS_TARGET_TOP_LEVEL :
        if (target->u.top_level.id) {
            *var_name = strcat_save(*var_name, target->u.top_level.id->name);
        }
        break;
    case VARASS_TARGET_STRUCTURE_MEMBER :
        config_process_target_part(
            target->u.struct_member.parent, var_name, num_indices, indices
        );
        *var_name = strcat_save(*var_name, "_");
        *var_name = strcat_save(*var_name, target->u.struct_member.member->name);
        break;
    case VARASS_TARGET_ARRAY_MEMBER :
        config_process_target_part(
            target->u.array_member.parent, var_name, num_indices, indices
        );
        *var_name = strcat_save(*var_name, "_%");
        *indices = (struct varass_eval_index_t *)
            safe_realloc(*indices, (*num_indices + 1) * sizeof(struct varass_eval_index_t));
        (*indices)[*num_indices].index = target->u.array_member.index;
        (*indices)[*num_indices].resolved = FALSE;
        ++*num_indices;
        break;
    }
}

static struct var_instance_t *
config_create_var_instance(
    struct var_scope_t *scope,
    struct location_t *location,
    char const *var_name,
    int num_indices,
    struct varass_eval_index_t *indices
)
{
    struct var_t *var = var_try_get(var_name);
    if (!var) {
        struct var_instance_t *inst
            = var_instance_create_from_string(scope, var_name);
        if (inst) {
            return inst;
        }
        else {
            char *locstr = location_toString(location);
            log_error(
                "Variable '%s' set at %s does not exist.\n",
                var_name,
                locstr
            );
            free(locstr);
            return NULL;
        }
    }
    else {
        struct var_instance_t *inst;
        int i;

        unsigned *real_indices
            = (unsigned *) safe_malloc(num_indices * sizeof(unsigned));

        for (i = 0; i < num_indices; ++i) {
            if (!indices[i].resolved) {
                switch (indices[i].index->kind) {
                case VARASS_INDEX_ABSOLUTE :
                    indices[i].abs_index = indices[i].index->u.absolute.number - 1;
                    break;
                case VARASS_INDEX_NEXT_FREE :
                    inst = var_instance_create(var, i, real_indices);
                    indices[i].abs_index = var_instance_get_next_free_index(inst);
                    var_instance_destroy(inst);
                    break;
                }

                indices[i].resolved = TRUE;
            }

            real_indices[i] = indices[i].abs_index;
        }

        inst = var_instance_create(var, num_indices, real_indices);
        free(real_indices);
        return inst;
    }
}

static BOOL
config_process_single_assignment(
    struct var_scope_t *scope,
    struct varass_t *ass,
    char const *var_name,
    int num_indices,
    struct varass_eval_index_t *indices
)
{
    struct location_t *loc;
    slot_flags_t *flags;
    struct var_slot_extra_t *extra;
    struct var_instance_t *var_inst
        = config_create_var_instance(scope, ass->location, var_name, num_indices, indices);

    if (!var_inst) {
        return FALSE;
    }

    loc = location_clone(ass->location);
    flags = create_slot_flags();
    extra = var_slot_extra_create(flags, free);
    if (!var_instance_set_value(var_inst, ass->value->value, extra, VAR_ACCESS_STRONG, ass->priority)) {
        char *locstr = location_toString(ass->location);
        char *slotname = var_instance_to_string(var_inst);
        log_error(
            "Setting variable '%s' to '%s' at %s failed.\n",
            slotname,
            ass->value->value,
            locstr
        );
        free(slotname);
        free(locstr);
        var_slot_extra_destroy(extra);
        location_destroy(loc);
        var_instance_destroy(var_inst);
        return FALSE;
    }

    var_instance_destroy(var_inst);
    return TRUE;
}

static BOOL
config_process_single_assignment_and_nested_assignments(
    struct var_scope_t *scope,
    struct varass_t *ass,
    char *prefix,
    int prefix_num_indices,
    struct varass_eval_index_t *prefix_indices
)
{
    struct varass_list_node_t *node;
    BOOL result = TRUE;

    char *var_name = strsave(prefix);
    int num_indices = prefix_num_indices;
    struct varass_eval_index_t *indices = (struct varass_eval_index_t *)
            safe_malloc(num_indices * sizeof(struct varass_eval_index_t));
    memcpy(indices, prefix_indices, num_indices * sizeof(struct varass_eval_index_t));

    config_process_target_part(ass->target, &var_name, &num_indices, &indices);

    if (ass->value) {
        if (!config_process_single_assignment(scope, ass, var_name, num_indices, indices)) {
            result = FALSE;
        }
    }

    node = ass->nested_assignments->first;
    while (node) {
        if (!config_process_single_assignment_and_nested_assignments(scope, node->assignment, var_name, num_indices, indices)) {
            result = FALSE;
        }
        node = node->next;
    }

    memcpy(prefix_indices, indices, prefix_num_indices * sizeof(struct varass_eval_index_t));
    free(indices);
    free(var_name);
    return result;
}

BOOL
config_process_assignments(void)
{
    BOOL result = TRUE;
    struct var_scope_t *const scope = var_get_global_scope();

    ARRAY_ITER(it, tree->assignments, ass, struct varass_t) {
        if (!config_process_single_assignment_and_nested_assignments(scope, ass, NULL, 0, NULL)) {
            result = FALSE;
        }
    }
    return result && tree->ok;
}

void
var_add_weak_declaration(char const *package, char const *name, char const *value,
                         char const *comment, char const *set_var,
                         int type, char const *file, int line,
                         int log_level, int filetype)
{
    char *upper_name;
    struct vartype_t *vartype;
    struct var_t *var;
    struct package_t *pkg;
    struct package_file_t *pkgfile;
    struct location_t *location;
    struct var_properties_t *props;
    struct var_instance_t *inst;
    struct var_slot_extra_t *extra;

    switch (type) {
    case TYPE_NUMERIC :
        vartype = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
        break;
    case TYPE_UNKNOWN:
        vartype = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
        break;
    default:
        fatal_exit("(%s:%d) unknown type %x\n",
                   __FILE__, __LINE__, type);
    }

    upper_name = strsave(convert_to_upper(name));
    var = var_try_get(upper_name);
    if (var) {
        var_remove(var);
    }

    pkg = package_get_or_add(package);
    pkgfile = package_get_file(pkg, filetype);
    location = location_create(pkgfile, line, 0);
    props = var_properties_create();
    if (comment) {
        var_properties_set_comment(props, comment);
    }
    var_properties_set_extra(
        props,
        var_extra_create(create_var_flags(TRUE, set_var), clone_var_flags, free_var_flags)
    );
    var = var_add(var_get_global_scope(), upper_name, vartype, VAR_PRIORITY_SCRIPT_TRANSIENT, location, props);
    free(upper_name);
    if (!var) {
        fatal_exit("(%s:%d) Cannot create variable '%s'", file, line, name);
    }

    inst = var_instance_create(var, 0, NULL);
    extra = var_slot_extra_create(create_slot_flags(), free);
    if (!var_instance_set_and_check_value(inst, value, extra, VAR_ACCESS_WEAK,
                                          VAR_PRIORITY_SCRIPT_TRANSIENT)) {
        var_slot_extra_destroy(extra);
        log_error("(%s:%d) Error setting variable '%s' to '%s'", file, line,
                  name, value);
    }
    var_instance_destroy(inst);

    UNUSED(log_level);
}

int
mark_var_requested(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(),
                                          convert_to_upper(name));
    if (!inst) {
        return ERR;
    }
    else {
        slot_flags_t *flags = get_variable_slot_extra(inst);
        var_instance_destroy(inst);
        if (flags) {
            flags->requested = TRUE;
            return OK;
        }
        else {
            fatal_exit("(%s:%d) variable %s misses extra data\n",
                       __FILE__, __LINE__, name);
        }
    }
}

int
mark_var_copied(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(),
                                          convert_to_upper(name));
    if (!inst) {
        return ERR;
    }
    else {
        slot_flags_t *flags = get_variable_slot_extra(inst);
        var_instance_destroy(inst);
        if (flags) {
            flags->copied = TRUE;
            return OK;
        }
        else {
            fatal_exit("(%s:%d) variable %s misses extra data\n",
                       __FILE__, __LINE__, name);
        }
    }
}

BOOL
is_var_tagged(char const *name)
{
    BOOL ret = FALSE;

    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(),
                                          convert_to_upper(name));
    if (!inst) {
        return FALSE;
    }

    if (var_get_priority(inst->var) >= VAR_PRIORITY_CONFIGURATION) {
        struct var_value_t *value = var_instance_try_get_value(inst);
        if (value != NULL) {
            var_value_destroy(value);
            ret = TRUE;
        }
    }

    var_instance_destroy(inst);
    return ret;
}

BOOL
is_var_numeric(char const *name)
{
    BOOL ret;

    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(),
                                          convert_to_upper(name));
    if (!inst) {
        return FALSE;
    }

    ret = var_get_type(inst->var)
            == vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    var_instance_destroy(inst);
    return ret;
}

BOOL
is_var_weak(char const *name)
{
    BOOL ret;
    struct var_value_t *value;
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), convert_to_upper(name));
    if (!inst) {
        return FALSE;
    }

    value = var_instance_try_get_value(inst);
    ret = value && value->priority == VAR_PRIORITY_SCRIPT_TRANSIENT;
    var_value_destroy(value);
    var_instance_destroy(inst);
    return ret;
}

BOOL
is_var_generated(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), convert_to_upper(name));
    if (!inst) {
        return FALSE;
    }
    else if (!var_instance_is_enabled(inst)) {
        var_instance_destroy(inst);
        return FALSE;
    }
    else {
        var_flags_t const *flags = get_var_flags(var_get_properties(inst->var));
        var_instance_destroy(inst);
        return flags ? flags->generated : FALSE;
    }
}

BOOL
is_var_copy_pending(char const *name)
{
    struct var_instance_t *inst
        = var_instance_create_from_string(var_get_global_scope(), convert_to_upper(name));
    if (!inst) {
        return FALSE;
    }
    else if (!var_instance_is_enabled(inst)) {
        var_instance_destroy(inst);
        return FALSE;
    }
    else {
        slot_flags_t *flags = get_variable_slot_extra(inst);
        var_instance_destroy(inst);
        if (flags) {
            return flags ? flags->requested && !flags->copied : FALSE;
        }
        else {
            return FALSE;
        }
    }
}
