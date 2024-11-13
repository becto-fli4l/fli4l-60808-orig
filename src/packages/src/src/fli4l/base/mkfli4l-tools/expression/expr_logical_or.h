/*****************************************************************************
 *  @file expression/expr_logical_or.h
 *  Functions for processing logical OR expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_LOGICAL_OR_H_
#define MKFLI4L_EXPRESSION_EXPR_LOGICAL_OR_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

struct expr_logical_or_t;

/**
 * Creates a logical OR expression which has short-circuit (or lazy)
 * semantics, i.e. in the expression arg1 && arg2, arg2 is not evaluated if
 * arg1 evaluates to TRUE.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param arg1
 *  The first argument. Ownership is transferred to the logical OR expression.
 * @param arg2
 *  The second argument. Ownership is transferred to the logical OR expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_logical_or_t *
expr_logical_or_create(
    struct var_scope_t *scope,
    struct expr_t *arg1,
    struct expr_t *arg2
);

/**
 * Casts a logical OR expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_logical_or_to_expr(struct expr_logical_or_t *expr);

/**
 * Returns the left argument of a logical OR expression.
 *
 * @param expr
 *  The logical OR expression.
 * @return
 *  The left argument of the expression.
 */
struct expr_t *
expr_logical_or_get_left_arg(struct expr_logical_or_t *expr);

/**
 * Returns the right argument of a logical OR expression.
 *
 * @param expr
 *  The logical OR expression.
 * @return
 *  The right argument of the expression.
 */
struct expr_t *
expr_logical_or_get_right_arg(struct expr_logical_or_t *expr);

#endif
