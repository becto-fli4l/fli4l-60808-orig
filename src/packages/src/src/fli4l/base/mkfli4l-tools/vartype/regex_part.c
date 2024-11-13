/*****************************************************************************
 *  @file vartype/regex_part.c
 *  Functions for parsing and assembling regular expressions associated with
 *  variable types and variable type extensions.
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
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "regex_part.h"

#define MODULE "vartype::regex_part"

/**
 * @name Types of expression parts.
 * @{
 */
/**
 * Represents an expression part being a string.
 */
#define VARTYPE_EXPR_STRING 1
/**
 * Represents an expression part being a reference to a variable type.
 */
#define VARTYPE_EXPR_REF 2
/**
 * @}
 */

/**
 * String marking the beginning of a reference to a variable type.
 */
#define VARTYPE_REF_START "(RE:"
/**
 * String marking the end of a reference to a variable type.
 */
#define VARTYPE_REF_END ")"

/**
 * Represents a part of a regular expression. The parts are hold in a reversed
 * linked list for simplicity.
 */
struct vartype_regex_part_t {
    /**
     * The type of the expression part.
     */
    int type;

    /**
     * The contents of the expression part.
     */
    union {
        /**
         * The string if type == VARTYPE_EXPR_STRING.
         */
        char *str;
        /**
         * The reference if type == VARTYPE_EXPR_REF.
         */
        struct vartype_t *ref;
    } contents;

    /**
     * Pointer to the next expression part, if any. May be NULL.
     */
    struct vartype_regex_part_t *next;
};

/**
 * Creates an expression from a string.
 *
 * @param last
 *  The last expression part parsed.
 * @param start
 *  The start of the string.
 * @param end
 *  The end of the string, pointing to the first character outside the
 *  expression part.
 * @param force
 *  If FALSE, empty strings (i.e. start == end) are ignored. If TRUE, an
 *  expression part describing the empty string is created and returned.
 * @return
 *  The new expression part or "last" if no object needed to be created.
 */
static struct vartype_regex_part_t *
vartype_regex_add_string(
    struct vartype_regex_part_t *last,
    char const *start,
    char const *end,
    BOOL force
)
{
    if (start != end || force) {
        struct vartype_regex_part_t *part
            = (struct vartype_regex_part_t *) safe_malloc (
                sizeof(struct vartype_regex_part_t)
            );
        part->type = VARTYPE_EXPR_STRING;
        part->contents.str = substrsave(start, end);
        part->next = last;
        return part;
    }
    else {
        return last;
    }
}

/**
 * Creates an expression part from a reference to a variable type.
 *
 * @param last
 *  The last expression part parsed.
 * @param start
 *  The start of the variable type's name.
 * @param end
 *  The end of the variable type's name, pointing to the first character not
 *  part of the name.
 * @return
 *  The new expression part.
 */
static struct vartype_regex_part_t *
vartype_regex_add_ref(
    struct vartype_regex_part_t *last,
    char const *start,
    char const *end
)
{
    struct vartype_regex_part_t *part;

    char *name = substrsave(start, end);
    struct vartype_t *vartype = vartype_get_or_add(name);
    free(name);

    part = (struct vartype_regex_part_t *) safe_malloc (
        sizeof(struct vartype_regex_part_t)
    );
    part->type = VARTYPE_EXPR_REF;
    part->contents.ref = vartype;
    part->next = last;
    return part;
}

struct vartype_regex_part_t *
vartype_regex_parse(char const *expr)
{
    struct vartype_regex_part_t *last = NULL;
    char const *start = expr;
    char const *next;
    int const len = strlen(expr);

    while ((next = strstr(start, VARTYPE_REF_START)) != NULL) {
        char const *end = strstr(next, VARTYPE_REF_END);
        if (end) {
            last = vartype_regex_add_string(last, start, next, FALSE);
            last = vartype_regex_add_ref(
                last, next + sizeof VARTYPE_REF_START - 1, end
            );
            start = end + 1;
        }
        else {
            break;
        }
    }

    return vartype_regex_add_string(last, start, expr + len, TRUE);
}

char *
vartype_regex_assemble(
    struct vartype_regex_part_t *THIS,
    BOOL expand
)
{
    char *result = NULL;

    if (THIS->next) {
        result = vartype_regex_assemble(THIS->next, expand);
        if (!result) {
            return NULL;
        }
    }

    switch (THIS->type) {
    case VARTYPE_EXPR_STRING :
        result = strcat_save(result, THIS->contents.str);
        break;

    case VARTYPE_EXPR_REF :
        if (expand) {
            char *regex = vartype_get_regex(THIS->contents.ref, TRUE);
            if (regex) {
                result = strcat_save(result, "(");
                result = strcat_save(result, regex);
                result = strcat_save(result, ")");
                free(regex);
            }
            else {
                free(result);
                result = NULL;
            }
        }
        else {
            result = strcat_save(result, VARTYPE_REF_START);
            result = strcat_save(result, THIS->contents.ref->name);
            result = strcat_save(result, VARTYPE_REF_END);
        }
        break;

    default :
        my_assert(FALSE);
    }

    return result;
}

void
vartype_regex_free(struct vartype_regex_part_t *THIS)
{
    if (THIS->next) {
        vartype_regex_free(THIS->next);
    }

    switch (THIS->type) {
    case VARTYPE_EXPR_STRING :
        free(THIS->contents.str);
        break;
    }

    free(THIS);
}
