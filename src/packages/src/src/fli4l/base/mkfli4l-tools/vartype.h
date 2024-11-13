/*****************************************************************************
 *  @file vartype.h
 *  Functions for processing variable types.
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

#ifndef MKFLI4L_VARTYPE_H_
#define MKFLI4L_VARTYPE_H_

#include <package.h>
#include <regex.h>

struct vartype_t;
struct expr_t;

/**
 * @name Names of predefined variable types.
 * @{
 */
/**
 * Predefined variable type accepting any value.
 */
#define VARTYPE_PREDEFINED_ANYTHING "NONE"
/**
 * Predefined variable type accepting unsigned integer values.
 */
#define VARTYPE_PREDEFINED_UNSIGNED_INTEGER "NUMERIC"
/**
 * Predefined variable type accepting boolean values.
 */
#define VARTYPE_PREDEFINED_BOOLEAN "YESNO"
/**
 * @}
 */

/**
 * @name Values of predefined variable types.
 * @{
 */
/**
 * Predefined value representing boolean TRUE.
 */
#define VALUE_BOOLEAN_TRUE "yes"
/**
 * Predefined value representing boolean FALSE.
 */
#define VALUE_BOOLEAN_FALSE "no"
/**
 * @}
 */

/**
 * Initializes this module.
 */
void
vartype_init_module(void);

/**
 * Finalizes this module.
 */
void
vartype_fini_module(void);

/**
 * Looks up a named variable type. Note that the resulting type (if any) will
 * not be compiled if not already done so.
 *
 * @param name
 *  The name of the variable type.
 * @return
 *  A pointer to the corresponding vartype_t object or NULL if not found.
 */
struct vartype_t *
vartype_get(char const *name);

/**
 * Looks up a named variable type. Note that the resulting type (if any) will
 * not be compiled if not already done so. Does not log an error if type does
 * not exist.
 *
 * @param name
 *  The name of the variable type.
 * @return
 *  A pointer to the corresponding vartype_t object or NULL if not found.
 */
struct vartype_t *
vartype_try_get(char const *name);

/**
 * Retrieves a variable type by name. If no such type exists, one placeholder
 * type is created without an expression, error message, or location. Such a
 * placeholder type is not compiled and must later be replaced as soon as its
 * real definition is encountered. If no such definition is provided, an error
 * will be generated at the time of calling vartype_finalize_type_system().
 *
 * @param name
 *  The name of the variable type to get or create.
 * @return
 *  The requested variable type.
 */
struct vartype_t *
vartype_get_or_add(char const *name);

/**
 * Adds a new variable type.
 *
 * @param name
 *  The name of the type. If NULL, the type is anonymous. An anonymous type
 *  cannot be looked up by vartype_get() or vartype_try_get().
 * @param expr
 *  The regular expression associated with this type. It may be NULL to force
 *  the creation of a temporary placeholder type, but only if the type is not
 *  anonymous at the same time.
 * @param error_msg
 *  The error message associated with this type.
 * @param comment
 *  A comment associated with this type.
 * @param location
 *  The location of the type definition. Ownership of this location_t object
 *  is passed to this module if the function succeeds.
 * @return
 *  A pointer to the new vartype_t object describing the type or NULL if an
 *  error occurred. Note that the variable type returned has not yet been
 *  compiled due to extensions that may be attached at a later point.
 */
struct vartype_t *
vartype_add(
    char const *name,
    char const *expr,
    char const *error_msg,
    char const *comment,
    struct location_t *location
);

/**
 * Creates a new temporary variable type. A temporary variable type is unnamed,
 * is not registered with any container, and is always compiled before being
 * returned.
 *
 * @param expr
 *  The regular expression associated with this type.
 * @param location
 *  The location of the type definition. Ownership of this location_t object
 *  is passed to this module if the function succeeds.
 * @param incomplete
 *  If TRUE, the expression will be prefixed by ^( and suffixed by )$.
 * @return
 *  A pointer to the new vartype_t object describing the type or NULL if an
 *  error occurred.
 */
struct vartype_t *
vartype_add_temporary(
    char const *expr,
    struct location_t *location,
    BOOL incomplete
);

/**
 * Extends an existing variable type. If the extension fails for some reason
 * (e.g. because the variable type has already been compiled), FALSE is
 * returned.
 *
 * @param THIS
 *  The type to be extended.
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
 *  The location of this type extension's definition. Ownership of this
 *  location_t object is passed to this module if the function succeeds.
 * @return
 *  TRUE if the type extension could be registered successfully, FALSE
 *  otherwise.
 */
BOOL
vartype_extend(
    struct vartype_t *THIS,
    char const *expr,
    struct expr_t *condition,
    char const *error_msg,
    char const *comment,
    struct location_t *location
);

/**
 * Removes a temporary variable type.
 *
 * @param THIS
 *  The variable type to remove.
 */
void
vartype_remove(struct vartype_t *THIS);

/**
 * Returns a variable type's name.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The name of the variable type or NULL if it is an anonymous type.
 */
char const *
vartype_get_name(struct vartype_t *THIS);

/**
 * Returns the regular expression associated with a variable type.
 *
 * @param THIS
 *  The variable type.
 * @param expand
 *  If TRUE, all embedded regular expressions are expanded recursively. If
 *  a cycle is detected, this function fails.
 * @return
 *  A dynamically allocated string containing the regular expression associated
 *  with passed variable type or NULL if expansion failed.
 */
char *
vartype_get_regex(struct vartype_t *THIS, BOOL expand);

/**
 * Returns a variable type's error message as found at its definition.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The error message.
 */
char const *
vartype_get_error_message(struct vartype_t *THIS);

/**
 * Returns a variable type's complete error message, being a concatenation of
 * the type's own error message and the error messages of all of its type
 * extensions.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The complete error message.
 */
char const *
vartype_get_complete_error_message(struct vartype_t *THIS);

/**
 * Returns a variable type's comment as found at its definition.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The comment.
 */
char const *
vartype_get_comment(struct vartype_t *THIS);

/**
 * Returns a variable type's complete comment, being a concatenation of the
 * type's own comment and the comments of all of its type extensions,
 * concatenated by a newline character.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The complete comment.
 */
char const *
vartype_get_complete_comment(struct vartype_t *THIS);

/**
 * Returns the expression type associated with a variable type. This mapping
 * depends on the names of the variable types and is not configurable.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The associated expression type.
 */
int
vartype_get_expression_type(struct vartype_t *THIS);

/**
 * Returns a variable type's location.
 *
 * @param THIS
 *  The variable type.
 * @return
 *  The location.
 */
struct location_t *
vartype_get_location(struct vartype_t *THIS);

/**
 * Checks whether a string value is correctly typed.
 *
 * @param THIS
 *  The variable type to check against.
 * @param value
 *  The value to check.
 * @return
 *  TRUE if the value is correctly typed (i.e. if it matches the underlying
 *  regular expression), FALSE otherwise.
 */
BOOL
vartype_match(struct vartype_t *THIS, char const *value);

/**
 * Checks whether a string value is correctly typed. Additionally, matched
 * subexpressions are recorded.
 *
 * @param THIS
 *  The variable type to check against.
 * @param value
 *  The value to check.
 * @return
 *  An array of matched subexpressions if the value is correctly typed (i.e. if
 *  it matches the underlying regular expression), NULL otherwise. If the value
 *  is correctly typed (and a valid array is therefore returned), its last
 *  entry has rm_so and rm_eo set to -1, so the array returned has at least one
 *  entry.
 */
regmatch_t *
vartype_match_ext(struct vartype_t *THIS, char const *value);

/**
 * Call this function to integrate all type extensions and to compile all
 * resulting regular expressions.
 */
BOOL
vartype_finalize_type_system(void);

#endif
