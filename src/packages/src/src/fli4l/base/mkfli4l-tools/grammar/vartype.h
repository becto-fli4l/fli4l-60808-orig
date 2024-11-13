/*****************************************************************************
 *  @file grammar/vartype.h
 *  Functions for scanning/parsing files containing variable types.
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

#ifndef MKFLI4L_GRAMMAR_VARTYPE_H_
#define MKFLI4L_GRAMMAR_VARTYPE_H_

#include "libmkfli4l/defs.h"
#include "libmkfli4l/array.h"
#include "grammar/common.h"
#include "grammar/expr.h"
#include "package.h"

/**
 * @name Definitions
 * @{
 */

/**
 * Represents a definition of a variable type.
 */
struct vartype_def_t {
    /**
     * The identifier.
     */
    struct identifier_t *id;
    /**
     * The associated regular expression.
     */
    struct qstr_t *regex;
    /**
     * The associated error message.
     */
    struct qstr_t *errmsg;
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
 * Creates a variable type definition.
 *
 * @param id
 *  The identifier of the variable type to define.
 * @param regex
 *  The associated regular expression.
 * @param errmsg
 *  The associated error message.
 * @param comment
 *  The associated comment.
 * @param location
 *  The location of the definition.
 * @return
 *  A vartype_def_t object. Use vartype_def_destroy() to free it after use.
 */
struct vartype_def_t *
vartype_def_create(
    struct identifier_t *id,
    struct qstr_t *regex,
    struct qstr_t *errmsg,
    char *comment,
    struct location_t *location
);

/**
 * Destroys a variable type definition.
 *
 * @param def
 *  The variable type definition to destroy. May be NULL.
 */
void
vartype_def_destroy(struct vartype_def_t *def);

/**
 * @}
 */

/**
 * @name Extensions
 * @{
 */

/**
 * Represents an extension of a variable type.
 */
struct vartype_ext_t {
    /**
     * The identifier.
     */
    struct identifier_t *id;
    /**
     * The condition used for enabling the extension. May be NULL.
     */
    struct expression_t *cond;
    /**
     * The associated regular expression.
     */
    struct qstr_t *regex;
    /**
     * The associated error message.
     */
    struct qstr_t *errmsg;
    /**
     * The associated comment.
     */
    char *comment;
    /**
     * The location at which this extension is to be found.
     */
    struct location_t *location;
    /**
     * TRUE if extension is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a variable type extension.
 *
 * @param id
 *  The identifier of the variable type to extend.
 * @param cond
 *  The condition used for enabling the extension. May be NULL.
 * @param regex
 *  The associated regular expression.
 * @param errmsg
 *  The associated error message.
 * @param comment
 *  The associated comment.
 * @param location
 *  The location of the definition.
 * @return
 *  A vartype_ext_t object. Use vartype_ext_destroy() to free it after use.
 */
struct vartype_ext_t *
vartype_ext_create(
    struct identifier_t *id,
    struct expression_t *cond,
    struct qstr_t *regex,
    struct qstr_t *errmsg,
    char *comment,
    struct location_t *location
);

/**
 * Destroys a variable type extension.
 *
 * @param ext
 *  The variable type extension to destroy. May be NULL.
 */
void
vartype_ext_destroy(struct vartype_ext_t *ext);

/**
 * @}
 */

/**
 * @name Parse tree
 * @{
 */

/**
 * Represents the results of parsing the variable type files.
 */
struct vartype_parse_tree_t {
    /**
     * TRUE if parse was successful, FALSE otherwise.
     */
    BOOL ok;
    /**
     * The variable type definitions parsed.
     */
    array_t *definitions;
    /**
     * The variable type extensions parsed.
     */
    array_t *extensions;
};

/**
 * Creates a parse tree structure for variable type files.
 *
 * @return
 *  A vartype_parse_tree_t object. Use vartype_parse_tree_destroy() to free it
 *  after use.
 */
struct vartype_parse_tree_t *
vartype_parse_tree_create(void);

/**
 * Adds a variable type definition to the parse tree.
 *
 * @param tree
 *  The parse tree.
 * @param def
 *  The variable type definition to add.
 */
void
vartype_parse_tree_add_definition(
    struct vartype_parse_tree_t *tree,
    struct vartype_def_t *def
);

/**
 * Adds a variable type extension to the parse tree.
 *
 * @param tree
 *  The parse tree.
 * @param ext
 *  The variable type extension to add.
 */
void
vartype_parse_tree_add_extension(
    struct vartype_parse_tree_t *tree,
    struct vartype_ext_t *ext
);

/**
 * Destroys a parse tree structure for variable type files.
 *
 * @param tree
 *  The parse tree to destroy.
 */
void
vartype_parse_tree_destroy(struct vartype_parse_tree_t *tree);

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
vartype_parse(
    void *scanner,
    struct package_file_t *file,
    struct vartype_parse_tree_t *tree
);

#endif
