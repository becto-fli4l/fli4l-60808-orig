/*****************************************************************************
 *  @file var/part.c
 *  Functions for processing variable parts.
 *
 *  Copyright (c) 2016 - fli4l-Team <team@fli4l.de>
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
#include <ctype.h>
#include <var.h>

#define VAR_PART_STRUCTURE_SEPARATOR_MODERN_STRING "."
#define VAR_PART_STRUCTURE_SEPARATOR_CLASSIC '_'
#define VAR_PART_STRUCTURE_SEPARATOR_CLASSIC_STRING "_"

#define VAR_PART_ARRAY_NAME_MODERN_LEFT "["
#define VAR_PART_ARRAY_NAME_MODERN_RIGHT "]"
#define VAR_PART_ARRAY_NAME_MODERN \
            VAR_PART_ARRAY_NAME_MODERN_LEFT VAR_PART_ARRAY_NAME_MODERN_RIGHT
#define VAR_PART_ARRAY_NAME_CLASSIC "%"

#define MODULE "var::part"

struct var_part_t *
var_part_create_top_level(char const *const name)
{
    struct var_part_t *part
        = (struct var_part_t *) safe_malloc(sizeof(struct var_part_t));
    part->kind = VAR_PART_TOP_LEVEL;
    part->u.top_level.name = strsave(name);
    return part;
}

struct var_part_t *
var_part_create_struct(struct var_part_t *const parent, char const *const name)
{
    struct var_part_t *part
        = (struct var_part_t *) safe_malloc(sizeof(struct var_part_t));
    part->kind = VAR_PART_STRUCTURE;
    part->u.structure.parent = parent;
    part->u.structure.name = strsave(name);
    return part;
}

struct var_part_t *
var_part_create_array(struct var_part_t *const parent, unsigned const index)
{
    struct var_part_t *part
        = (struct var_part_t *) safe_malloc(sizeof(struct var_part_t));
    part->kind = VAR_PART_ARRAY;
    part->u.array.parent = parent;
    part->u.array.index = index;
    return part;
}

void
var_part_destroy(struct var_part_t *const THIS)
{
    switch (THIS->kind) {
    case VAR_PART_TOP_LEVEL :
        free(THIS->u.top_level.name);
        break;
    case VAR_PART_STRUCTURE :
        var_part_destroy(THIS->u.structure.parent);
        free(THIS->u.structure.name);
        break;
    case VAR_PART_ARRAY :
        var_part_destroy(THIS->u.array.parent);
        break;
    default :
        my_assert(FALSE);
        break;
    }
    free(THIS);
}

char *
var_part_to_string(struct var_part_t *const THIS)
{
    switch (THIS->kind) {
    case VAR_PART_TOP_LEVEL :
        return strsave(THIS->u.top_level.name);
    case VAR_PART_STRUCTURE :
        return strcat_save(
            strcat_save(
                var_part_to_string(THIS->u.structure.parent),
                VAR_PART_STRUCTURE_SEPARATOR_MODERN_STRING
            ),
            THIS->u.structure.name
        );
    case VAR_PART_ARRAY :
        if (THIS->u.array.index == 0) {
            return strcat_save(
                var_part_to_string(THIS->u.array.parent),
                VAR_PART_ARRAY_NAME_MODERN
            );
        }
        else {
            char *s = safe_sprintf("%u", THIS->u.array.index);
            char *result = strcat_save(
                strcat_save(
                    strcat_save(
                        var_part_to_string(THIS->u.array.parent),
                        VAR_PART_ARRAY_NAME_MODERN_LEFT
                    ),
                    s
                ),
                VAR_PART_ARRAY_NAME_MODERN_RIGHT
            );
            free(s);
            return result;
        }
    default :
        my_assert(FALSE);
        return NULL;
    }
}

char *
var_part_to_string_classic_style(struct var_part_t *const THIS)
{
    switch (THIS->kind) {
    case VAR_PART_TOP_LEVEL :
        return strsave(THIS->u.top_level.name);
    case VAR_PART_STRUCTURE :
        return strcat_save(
            strcat_save(
                var_part_to_string_classic_style(THIS->u.structure.parent),
                VAR_PART_STRUCTURE_SEPARATOR_CLASSIC_STRING
            ),
            THIS->u.structure.name
        );
    case VAR_PART_ARRAY :
        if (THIS->u.array.index == 0) {
            return strcat_save(
                strcat_save(
                    var_part_to_string_classic_style(THIS->u.array.parent),
                    VAR_PART_STRUCTURE_SEPARATOR_CLASSIC_STRING
                ),
                VAR_PART_ARRAY_NAME_CLASSIC
            );
        }
        else {
            char *s = safe_sprintf("%u", THIS->u.array.index);
            char *result = strcat_save(
                strcat_save(
                    var_part_to_string_classic_style(THIS->u.array.parent),
                    VAR_PART_STRUCTURE_SEPARATOR_CLASSIC_STRING
                ),
                s
            );
            free(s);
            return result;
        }
    default :
        my_assert(FALSE);
        return NULL;
    }
}

struct var_part_t *
var_part_create_from_string_classic_style(char const *const name)
{
    struct var_part_t *last = NULL;

    size_t len = strlen(name);
    char *n = (char *) malloc(len + 2);
    char *p = n;
    char *q = n;

    strcpy(n, name);
    n[len] = VAR_PART_STRUCTURE_SEPARATOR_CLASSIC;
    n[len + 1] = '\0';

    while ((q = strchr(p, VAR_PART_STRUCTURE_SEPARATOR_CLASSIC)) != NULL) {
        *q = '\0';
        if (*p == '\0') {
            /* empty components are not supported */
            if (last != NULL) {
                var_part_destroy(last);
            }
            free(n);
            return NULL;
        }

        if (last == NULL) {
            last = var_part_create_top_level(p);
        }
        else if (isdigit(*p)) {
            last = var_part_create_array(last, atoi(p));
        }
        else if (strcmp(p, VAR_PART_ARRAY_NAME_CLASSIC) == 0) {
            last = var_part_create_array(last, 0);
        }
        else {
            last = var_part_create_struct(last, p);
        }

        p = q + 1;
    }

    free(n);
    return last;
}
