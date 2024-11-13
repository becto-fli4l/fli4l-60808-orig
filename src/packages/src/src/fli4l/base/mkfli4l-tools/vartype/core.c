/*****************************************************************************
 *  @file vartype/core.c
 *  Functions for processing variable types.
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
 *****************************************************************************
 */

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <expression.h>
#include <package.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "regex_part.h"
#include "regex.h"
#include "extension.h"

#define MODULE "vartype::core"

/**
 * Name prefix of integer variable types.
 */
#define VARTYPE_PREDEFINED_INTEGER_PREFIX "NUM"

struct hashmap_t *vartypes = NULL;
array_t *anon_vartypes = NULL;

/**
 * Creates a variable type.
 *
 * @param name
 *  The name of the type. If NULL, the type is anonymous. An anonymous type
 *  cannot be looked up by vartype_get() or vartype_try_get().
 * @param expr
 *  The regular expression associated with this type. It may be NULL to force
 *  the creation of a temporary placeholder type, but only if the type is not
 *  anonymous at the same time.
 * @param error_msg
 *  The error message associated with this type. May be NULL.
 * @param comment
 *  A comment associated with this type. May be NULL.
 * @param location
 *  The location of the type definition. Ownership of this location_t object
 *  is passed to this module if the function succeeds.
 * @return
 *  A pointer to the new vartype_t object describing the type.
 */
static struct vartype_t *
vartype_create(
    char const *name,
    struct vartype_regex_part_t *expr,
    char const *error_msg,
    char const *comment,
    struct location_t *location
)
{
    struct vartype_t *vartype;

    vartype = (struct vartype_t *) safe_malloc(sizeof(struct vartype_t));
    vartype->name = strsave(name);
    vartype->expr = expr;
    vartype->error_msg = strsave(error_msg);
    vartype->comment = strsave(comment);
    vartype->location = location;
    vartype->compiled_state = NULL;
    vartype->expansion_in_progress = FALSE;
    vartype->extensions = NULL;
    vartype->temporary = FALSE;
    vartype->predefined = FALSE;
    vartype->incomplete = TRUE;

    return vartype;
}

/**
 * Destroys a variable type.
 *
 * @param THIS
 *  The variable type to destroy.
 */
static void
vartype_destroy(struct vartype_t *THIS)
{
    free(THIS->name);
    if (THIS->expr) {
        vartype_regex_free(THIS->expr);
    }
    free(THIS->comment);
    free(THIS->error_msg);
    location_destroy(THIS->location);
    if (THIS->compiled_state) {
        vartype_free_compiled_state(THIS);
    }
    if (THIS->extensions) {
        vartype_extension_destroy(THIS->extensions);
    }
    free(THIS);
}

struct vartype_t *
vartype_get(char const *name)
{
    struct vartype_t *const vartype = vartype_try_get(name);
    if (vartype) {
        return vartype;
    }
    else {
        log_error(MODULE ": Variable type '%s' does not exist.\n", name);
        return NULL;
    }
}

struct vartype_t *
vartype_try_get(char const *name)
{
    return (struct vartype_t *) hashmap_get(vartypes, &name);
}

struct vartype_t *
vartype_get_or_add(char const *name)
{
    struct vartype_t *vartype = vartype_try_get(name);
    if (vartype) {
        return vartype;
    }
    else {
        return vartype_add(
            name,
            NULL,
            "",
            "",
            location_create_internal(PACKAGE_FILE_TYPE_VARTYPE)
        );
    }
}

struct vartype_t *
vartype_add(
    char const *name,
    char const *expr,
    char const *error_msg,
    char const *comment,
    struct location_t *location
)
{
    struct vartype_regex_part_t *e;
    struct vartype_t *vartype;

    my_assert(name != NULL || expr != NULL);
    my_assert(error_msg != NULL);
    my_assert(comment != NULL);

    e = expr ? vartype_regex_parse(expr) : NULL;
    vartype = name ? vartype_try_get(name) : NULL;
    if (!vartype) {
        vartype = vartype_create(name, e, error_msg, comment, location);
        if (name) {
            hashmap_put(vartypes, vartype);
        }
        else {
            append_new_elem(anon_vartypes, vartype);
        }

        return vartype;
    }
    else if (vartype->expr) {
        if (vartype->predefined) {
            char *regex = vartype_get_regex(vartype, FALSE);
            if (strcmp(expr, regex) == 0) {
                free(regex);
                vartype_regex_free(vartype->expr);
                free(vartype->comment);
                free(vartype->error_msg);
                location_destroy(vartype->location);
            }
            else {
                char *const locstr = location_toString(location);
                log_error(
                    MODULE ": Redefinition of predefined type '%s' with '%s' at %s not supported; the original regular expression is '%s'.\n",
                    name,
                    expr,
                    locstr,
                    regex
                );
                free(locstr);
                free(regex);
                vartype_regex_free(e);
                return NULL;
            }
        }
        else {
            char *const locstr = location_toString(location);
            char *const locstrOrig = location_toString(vartype->location);
            log_error(
                MODULE ": Redefinition of variable type '%s' at %s not supported; the original definition is at %s.\n",
                name,
                locstr,
                locstrOrig
            );
            free(locstrOrig);
            free(locstr);
            vartype_regex_free(e);
            return NULL;
        }
    }
    else {
        free(vartype->error_msg);
        free(vartype->comment);
        location_destroy(vartype->location);
    }

    vartype->expr = e;
    vartype->error_msg = strsave(error_msg);
    vartype->comment = strsave(comment);
    vartype->location = location;
    vartype->predefined = FALSE;
    return vartype;
}

struct vartype_t *
vartype_add_temporary(
    char const *expr,
    struct location_t *location,
    BOOL incomplete
)
{
    struct vartype_regex_part_t *e;
    struct vartype_t *vartype;

    my_assert(expr != NULL);

    e = vartype_regex_parse(expr);
    vartype = vartype_create(NULL, e, "", "", location);
    vartype->temporary = TRUE;
    vartype->incomplete = incomplete;
    if (vartype_compile(vartype)) {
        return vartype;
    }
    else {
        vartype_destroy(vartype);
        return NULL;
    }
}

BOOL
vartype_extend(
    struct vartype_t *THIS,
    char const *expr,
    struct expr_t *condition,
    char const *error_msg,
    char const *comment,
    struct location_t *location
)
{
    my_assert(expr != NULL);
    my_assert(error_msg != NULL);
    my_assert(comment != NULL);

    if (THIS->compiled_state) {
        char *const locstr = location_toString(location);
        char *const locstrOrig = location_toString(THIS->location);
        log_error(
            MODULE ": Extension of compiled variable type '%s' at %s not supported; the original definition is at %s.\n",
            THIS->name,
            locstr,
            locstrOrig
        );
        free(locstr);
        free(locstrOrig);
        return FALSE;
    }

    vartype_extension_create(
        expr, condition, error_msg, comment, location, &THIS->extensions
    );
    return TRUE;
}

void
vartype_remove(struct vartype_t *THIS)
{
    my_assert(THIS->temporary);
    vartype_destroy(THIS);
}

char const *
vartype_get_name(struct vartype_t *THIS)
{
    return THIS->name;
}

char const *
vartype_get_error_message(struct vartype_t *THIS)
{
    return THIS->error_msg;
}

char const *
vartype_get_comment(struct vartype_t *THIS)
{
    return THIS->comment;
}

int
vartype_get_expression_type(struct vartype_t *THIS)
{
    if (THIS->name) {
        if (strcmp(THIS->name, VARTYPE_PREDEFINED_BOOLEAN) == 0) {
            return EXPR_TYPE_BOOLEAN;
        }
        else if (strncmp(THIS->name,
                VARTYPE_PREDEFINED_INTEGER_PREFIX,
                sizeof (VARTYPE_PREDEFINED_INTEGER_PREFIX) - 1) == 0) {
            return EXPR_TYPE_INTEGER;
        }
    }

    return EXPR_TYPE_STRING;
}

char const *
vartype_get_complete_error_message(struct vartype_t *THIS)
{
    my_assert(THIS->compiled_state);
    return THIS->compiled_state->complete_error_msg;
}

char const *
vartype_get_complete_comment(struct vartype_t *THIS)
{
    my_assert(THIS->compiled_state);
    return THIS->compiled_state->complete_comment;
}

char *
vartype_get_regex(struct vartype_t *THIS, BOOL expand)
{
    char *result;

    if (!THIS->expr) {
        log_error(
            MODULE ": No definition found for variable type '%s'.\n",
            THIS->name
        );
        return NULL;
    }

    if (THIS->expansion_in_progress) {
        log_error(
            MODULE ": Variable type '%s' directly or indirectly references itself.\n",
            THIS->name
        );
        return NULL;
    }

    THIS->expansion_in_progress = TRUE;

    result = vartype_regex_assemble(THIS->expr, expand);
    if (result) {
        result = vartype_extension_append_regex(
            THIS->extensions,
            THIS,
            result,
            expand
        );
    }

    THIS->expansion_in_progress = FALSE;
    return result;
}

struct location_t *
vartype_get_location(struct vartype_t *THIS)
{
    return THIS->location;
}

/**
 * Destroys a variable type.
 *
 * @param entry
 *  The variable type to destroy. It is of type void* in order to be able to
 *  use this function as a hash map object destructor.
 */
static void
vartype_destroy_action(void *entry)
{
    struct vartype_t *const THIS = (struct vartype_t *) entry;
    vartype_destroy(THIS);
}

void
vartype_core_init(void)
{
    vartypes = hashmap_create(
        offsetof(struct vartype_t, name),
        1024,
        hash_ci_string,
        compare_ci_string_keys,
        vartype_destroy_action
    );
    anon_vartypes = init_array(64);
}

void
vartype_core_fini(void)
{
    ARRAY_ITER(it, anon_vartypes, vartype, struct vartype_t) {
        vartype_destroy(vartype);
    }
    free_array(anon_vartypes);
    anon_vartypes = NULL;
    hashmap_destroy(vartypes);
    vartypes = NULL;
}
