/*****************************************************************************
 *  @file expression/expr_equal.h
 *  Functions for processing equality comparison expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_EQUAL_H_
#define MKFLI4L_EXPRESSION_EXPR_EQUAL_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

struct expr_equal_t;

/**
 * Creates an equality comparison expression.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param arg1
 *  The first argument. Ownership is transferred to the expression.
 * @param arg2
 *  The second argument. Ownership is transferred to the expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_equal_t *
expr_equal_create(
    struct var_scope_t *scope,
    struct expr_t *arg1,
    struct expr_t *arg2
);

/**
 * Casts an equality comparison expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_equal_to_expr(struct expr_equal_t *expr);

/**
 * Returns the left argument of an equality comparison expression.
 *
 * @param expr
 *  The equality comparison expression.
 * @return
 *  The left argument of the expression.
 */
struct expr_t *
expr_equal_get_left_arg(struct expr_equal_t *expr);

/**
 * Returns the right argument of an equality comparison expression.
 *
 * @param expr
 *  The equality comparison expression.
 * @return
 *  The right argument of the expression.
 */
struct expr_t *
expr_equal_get_right_arg(struct expr_equal_t *expr);

#endif
