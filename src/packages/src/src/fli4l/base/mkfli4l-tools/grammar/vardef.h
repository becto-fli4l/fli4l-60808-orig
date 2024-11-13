/*****************************************************************************
 *  @file grammar/vardef.h
 *  Functions for scanning/parsing files containing variable definitions.
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

#ifndef MKFLI4L_GRAMMAR_VARDEF_H_
#define MKFLI4L_GRAMMAR_VARDEF_H_

#include "libmkfli4l/defs.h"
#include "libmkfli4l/array.h"
#include "grammar/common.h"
#include "grammar/expr.h"
#include "package.h"

/**
 * @name Type references
 * @{
 */

/**
 * Represents a reference to a variable type.
 */
struct vardef_typeref_t {
    /**
     * Union of a named and an anonymous variable type.
     */
    union {
        /**
         * The name of an existing variable type.
         */
        struct identifier_t *id;
        /**
         * An ad-hoc regular expression.
         */
        char *regex;
    } u;
    /**
     * If TRUE, "vartype" contains the name of an existing variable type.
     * If FALSE, it contains an ad-hoc regular expression.
     */
    BOOL named;
    /**
     * TRUE if reference is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a reference to a named variable type.
 *
 * @param vartype
 *  The identifier of the variable type to use.
 * @return
 *  A vardef_typeref_t object. Use vardef_typeref_destroy() to free it after
 *  use.
 */
struct vardef_typeref_t *
vardef_typeref_create_named(struct identifier_t *vartype);

/**
 * Creates a reference to an unnamed variable type reference by value.
 *
 * @param vartype
 *  An ad-hoc regular expression.
 * @return
 *  A vardef_typeref_t object. Use vardef_typeref_destroy() to free it after
 *  use.
 */
struct vardef_typeref_t *
vardef_typeref_create_unnamed(char *vartype);

/**
 * Destroys a reference to a variable type.
 *
 * @param typeref
 *  The reference to destroy. May be NULL.
 */
void
vardef_typeref_destroy(struct vardef_typeref_t *typeref);

/**
 * @}
 */

/**
 * @name Definitions
 * @{
 */

/**
 * Represents a definition of a variable.
 */
struct vardef_t {
    /**
     * TRUE if variable is marked as optional.
     */
    BOOL optional;
    /**
     * The identifier.
     */
    struct identifier_t *id;
    /**
     * The associated condition (if any).
     */
    struct expression_t *cond;
    /**
     * The associated array bound variable (if any).
     */
    struct identifier_t *array_bound;
    /**
     * The associated variable type.
     */
    struct vardef_typeref_t *type;
    /**
     * The associated default value (if any).
     */
    struct expression_t *defvalue;
    /**
     * The associated comment.
     */
    char *comment;
    /**
     * The location at which this definition is to be found.
     */
    struct location_t *location;
    /**
     * TRUE if definition is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a variable definition.
 *
 * @param optional
 *  If TRUE, the variable is optional, otherwise it is mandatory.
 * @param id
 *  The identifier of the variable to define.
 * @param cond
 *  The associated enabling condition (if any).
 * @param array_bound
 *  The associated array bound variable (if any).
 * @param type
 *  The associated variable type.
 * @param defvalue
 *  The associated default value (if any).
 * @param comment
 *  The associated comment.
 * @param location
 *  The location of the definition.
 * @return
 *  A vardef_t object. Use vardef_destroy() to free it after use.
 */
struct vardef_t *
vardef_create(
    BOOL optional,
    struct identifier_t *id,
    struct expression_t *cond,
    struct identifier_t *array_bound,
    struct vardef_typeref_t *type,
    struct expression_t *defvalue,
    char *comment,
    struct location_t *location
);

/**
 * Destroys a variable definition.
 *
 * @param def
 *  The variable definition to destroy. May be NULL.
 */
void
vardef_destroy(struct vardef_t *def);

/**
 * @}
 */

/**
 * @name Template definitions
 * @{
 */

/**
 * Represents a definition of a variable template.
 */
struct vardef_template_t {
    /**
     * TRUE if variable template is marked as optional.
     */
    BOOL optional;
    /**
     * The identifier.
     */
    struct identifier_t *id;
    /**
     * The associated condition (if any).
     */
    struct expression_t *cond;
    /**
     * The associated array bound variable (if any).
     */
    struct identifier_t *array_bound;
    /**
     * The associated variable type.
     */
    struct vardef_typeref_t *type;
    /**
     * The associated default value (if any).
     */
    struct expression_t *defvalue;
    /**
     * The associated comment.
     */
    char *comment;
    /**
     * The location at which this definition is to be found.
     */
    struct location_t *location;
    /**
     * TRUE if definition is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a variable template definition.
 *
 * @param optional
 *  If TRUE, the variable template is optional, otherwise it is mandatory.
 * @param id
 *  The identifier of the variable template to define.
 * @param cond
 *  The associated enabling condition (if any).
 * @param array_bound
 *  The associated array bound variable (if any).
 * @param type
 *  The associated variable type.
 * @param defvalue
 *  The associated default value (if any).
 * @param comment
 *  The associated comment.
 * @param location
 *  The location of the definition.
 * @return
 *  A vardef_template_t object. Use vardef_destroy_template() to free it after use.
 */
struct vardef_template_t *
vardef_create_template(
    BOOL optional,
    struct identifier_t *id,
    struct expression_t *cond,
    struct identifier_t *array_bound,
    struct vardef_typeref_t *type,
    struct expression_t *defvalue,
    char *comment,
    struct location_t *location
);

/**
 * Destroys a variable template definition.
 *
 * @param def
 *  The variable template definition to destroy. May be NULL.
 */
void
vardef_destroy_template(struct vardef_template_t *def);

/**
 * @}
 */

/**
 * @name Parse tree
 * @{
 */

/**
 * Represents the results of parsing the variable definition files.
 */
struct vardef_parse_tree_t {
    /**
     * TRUE if parse was successful, FALSE otherwise.
     */
    BOOL ok;
    /**
     * The variable definitions parsed.
     */
    array_t *definitions;
    /**
     * The variable template definitions parsed.
     */
    array_t *template_definitions;
};

/**
 * Creates a parse tree structure for variable definition files.
 *
 * @return
 *  A vardef_parse_tree_t object. Use vardef_parse_tree_destroy() to free it
 *  after use.
 */
struct vardef_parse_tree_t *
vardef_parse_tree_create(void);

/**
 * Adds a variable definition to the parse tree.
 *
 * @param tree
 *  The parse tree.
 * @param def
 *  The variable definition to add.
 */
void
vardef_parse_tree_add_definition(
    struct vardef_parse_tree_t *tree,
    struct vardef_t *def
);

/**
 * Adds a variable template definition to the parse tree.
 *
 * @param tree
 *  The parse tree.
 * @param def
 *  The variable template definition to add.
 */
void
vardef_parse_tree_add_template_definition(
    struct vardef_parse_tree_t *tree,
    struct vardef_template_t *def
);

/**
 * Destroys a parse tree structure for variable definition files.
 *
 * @param tree
 *  The parse tree to destroy.
 */
void
vardef_parse_tree_destroy(struct vardef_parse_tree_t *tree);

/**
 * @}
 */

/**
 * The parser's entry point.
 *
 * @param scanner
 *  The scanner to use.
 * @param file
 *  The package file that is to be parsed.
 * @param tree
 *  The parse tree to fill.
 * @return
 *  Zero if successful, a non-zero value otherwise.
 */
int
vardef_parse(
    void *scanner,
    struct package_file_t *file,
    struct vardef_parse_tree_t *tree
);

#endif
