/*****************************************************************************
 *  @file expression/expr_match.h
 *  Functions for processing regular expression matching expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_MATCH_H_
#define MKFLI4L_EXPRESSION_EXPR_MATCH_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>
#include <vartype.h>

/**
 * Name of array receiving matched subexpressions.
 */
#define EXPR_MATCH_SUBEXP_ARRAY "MATCH_%"

struct expr_match_t;

/**
 * Creates a regular expression matching expression. Such an expression tries
 * to match a string against a regular expression. If evaluation succeeds,
 * the subexpressions that were matched are stored in the array MATCH_%.
 * Otherwise, FALSE is returned.
 *
 * Note that if storing subexpressions fails or any other error occurs while
 * evaluating a regular expression matching expression, an error value is
 * returned.
 *
 * Note that no subexpressions behind the first subexpression which has not
 * been matched are stored. That means that matching "a(bc)*(d)" with "ad" will
 * result in MATCH_N being 0 as the first subexpression has not been matched at
 * all. However, if a subexpression is matched by the empty string, it is
 * recorded, so matching "a(b*)(d)" with "ad" will set MATCH_N to 2, MATCH_1 to
 * the empty string, and MATCH_2 to "d".
 *
 * Note that references pointing to other variable types as in "a(RE:NUMERIC)b"
 * count as subexpressions.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param arg
 *  The argument to match. Ownership is transferred to the expression.
 * @param regex
 *  The regular expression to match against.
 * @return
 *  A pointer to a dynamically allocated expression.
 */
struct expr_match_t *
expr_match_create(
    struct var_scope_t *scope,
    struct expr_t *arg,
    struct expr_t *regex
);

/**
 * Casts a match expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_match_to_expr(struct expr_match_t *expr);

/**
 * Returns the left argument of a match expression.
 *
 * @param expr
 *  The match expression.
 * @return
 *  The left argument of the expression.
 */
struct expr_t *
expr_match_get_left_arg(struct expr_match_t *expr);

/**
 * Returns the right argument of a match expression.
 *
 * @param expr
 *  The match expression.
 * @return
 *  The right argument of the expression.
 */
struct expr_t *
expr_match_get_right_arg(struct expr_match_t *expr);

#endif
