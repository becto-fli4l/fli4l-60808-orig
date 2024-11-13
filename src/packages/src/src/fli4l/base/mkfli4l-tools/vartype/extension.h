/*****************************************************************************
 *  @file vartype/extension.h
 *  Functions for processing variable type extensions.
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

#ifndef MKFLI4L_VARIABLE_VARTYPE_EXTENSION_H_
#define MKFLI4L_VARIABLE_VARTYPE_EXTENSION_H_

#include <vartype.h>
#include <expression/expr.h>

struct vartype_extension_t;

/**
 * Creates a variable type extension.
 *
 * @param expr
 *  The regular expression associated with this type extension.
 * @param condition
 *  The condition associated with this type extension. Ownership of this object
 *  is passed to this module if the function succeeds. May be NULL; in this
 *  case, the expression used is boolean TRUE.
 * @param error_msg
 *  The error message associated with this type extension.
 * @param comment
 *  A comment associated with this type extension.
 * @param location
 *  The location of this type extension's definition.
 * @param list
 *  Pointer to the linked list containing variable type extensions. The new
 *  type extension is appended to this list.
 * @return
 *  A pointer to the new vartype_extension_t object describing the type
 *  extension.
 */
struct vartype_extension_t *
vartype_extension_create(
    char const *expr,
    struct expr_t *condition,
    char const *error_msg,
    char const *comment,
    struct location_t *location,
    struct vartype_extension_t **list
);

/**
 * Destroys a variable type extension.
 *
 * @param THIS
 *  The variable type extension to destroy.
 */
void
vartype_extension_destroy(struct vartype_extension_t *THIS);

/**
 * Appends the regular expressions of this and all following extensions to the
 * regular expression associated with a variable type.
 *
 * @param THIS
 *  The first variable type extension to process.
 * @param vartype
 *  The variable type.
 * @param expr
 *  The regular expression to append the extensions' expressions to.
 * @param expand
 *  If TRUE, all embedded regular expressions are expanded recursively. If
 *  a cycle is detected, this function fails.
 * @return
 *  A dynamically allocated string containing the resulting regular expression
 *  or NULL if expansion failed.
 */
char *
vartype_extension_append_regex(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype,
    char *expr,
    BOOL expand
);

/**
 * Concatenates the error messages of a variable type and and of all following
 * extensions associated with a variable type.
 *
 * @param THIS
 *  The first variable type extension to process.
 * @param vartype
 *  The variable type.
 * @return
 *  A dynamically allocated string containing the resulting error message or
 *  NULL if concatenation failed.
 */
char *
vartype_extension_assemble_error_message(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype
);

/**
 * Concatenates the comments of a variable type and and of all following
 * extensions associated with a variable type.
 *
 * @param THIS
 *  The first variable type extension to process.
 * @param vartype
 *  The variable type.
 * @return
 *  A dynamically allocated string containing the resulting comment or NULL if
 *  concatenation failed.
 */
char *
vartype_extension_assemble_comment(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype
);

#endif
