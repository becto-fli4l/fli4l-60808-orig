/*****************************************************************************
 *  @file expression/expr_logical_not.h
 *  Functions for processing logical NOT expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_LOGICAL_NOT_H_
#define MKFLI4L_EXPRESSION_EXPR_LOGICAL_NOT_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

struct expr_logical_not_t;

/**
 * Creates a logical NOT expression.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param arg
 *  The argument to negate. Ownership is transferred to the logical NOT
 *  expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_logical_not_t *
expr_logical_not_create(struct var_scope_t *scope, struct expr_t *arg);

/**
 * Casts a logical NOT expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_logical_not_to_expr(struct expr_logical_not_t *expr);

/**
 * Returns the argument of a logical NOT expression.
 *
 * @param expr
 *  The logical NOT expression.
 * @return
 *  The argument of the expression.
 */
struct expr_t *
expr_logical_not_get_arg(struct expr_logical_not_t *expr);

#endif
