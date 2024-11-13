/*****************************************************************************
 *  @file expression/expr_tobool.h
 *  Functions for processing TOBOOL expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_LOGICAL_TOBOOL_H_
#define MKFLI4L_EXPRESSION_EXPR_LOGICAL_TOBOOL_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

struct expr_tobool_t;

/**
 * Creates a TOBOOL expression. Its textual representation is [...]. A TOBOOL
 * expression takes one operand and converts it to a boolean value. The
 * peculiarity lies in the handling of error expressions: They are converted to
 * boolean FALSE. So variable
 *
 *   [VAR_A] || [VAR_B]
 *
 * work even if VAR_A is disabled (and reading it yields an error value).
 *
 * @param scope
 *  The scope this expression lives in.
 * @param arg
 *  The argument to negate. Ownership is transferred to the TOBOOL expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_tobool_t *
expr_tobool_create(struct var_scope_t *scope, struct expr_t *arg);

/**
 * Casts a TOBOOL expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_tobool_to_expr(struct expr_tobool_t *expr);

/**
 * Returns the argument of a TOBOOL expression.
 *
 * @param expr
 *  The TOBOOL expression.
 * @return
 *  The argument of the expression.
 */
struct expr_t *
expr_tobool_get_arg(struct expr_tobool_t *expr);

#endif
