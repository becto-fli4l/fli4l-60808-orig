/*****************************************************************************
 *  @file grammar/expr.h
 *  Functions for scanning/parsing expressions.
 *
 *  Copyright (c) 2015-2016 The fli4l team
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

#ifndef MKFLI4L_GRAMMAR_EXPR_H_
#define MKFLI4L_GRAMMAR_EXPR_H_

#include "libmkfli4l/defs.h"
#include "libmkfli4l/array.h"
#include "grammar/common.h"
#include "package.h"
#include "expression.h"

/**
 * @name Parse tree
 * @{
 */

/**
 * Represents the results of parsing an expression.
 */
struct expr_parse_tree_t {
    /**
     * TRUE if parse was successful, FALSE otherwise.
     */
    BOOL ok;
    /**
     * The expression parsed.
     */
    struct expr_t *expr;
};

/**
 * Creates a parse tree structure for expressions.
 *
 * @return
 *  An expr_parse_tree_t object. Use expr_parse_tree_destroy() to free it after
 * use.
 */
struct expr_parse_tree_t *
expr_parse_tree_create(void);

/**
 * Destroys a parse tree structure for expressions.
 *
 * @param tree
 *  The parse tree to destroy.
 */
void
expr_parse_tree_destroy(struct expr_parse_tree_t *tree);

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
 * @param scope
 *  The scope for looking up variables.
 * @return
 *  Zero if successful, a non-zero value otherwise.
 */
int
expr_parse(
    void *scanner,
    struct package_file_t *file,
    struct expr_parse_tree_t *tree,
    struct var_scope_t *scope
);

#endif
