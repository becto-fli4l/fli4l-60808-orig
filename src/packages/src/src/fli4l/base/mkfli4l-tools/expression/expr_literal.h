/*****************************************************************************
 *  @file expression/expr_literal.h
 *  Functions for processing literal expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_LITERAL_H_
#define MKFLI4L_EXPRESSION_EXPR_LITERAL_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

/**
 * Textual representation of the TRUE expression.
 */
#define EXPR_LITERAL_STRING_TRUE "yes"
/**
 * Textual representation of the FALSE expression.
 */
#define EXPR_LITERAL_STRING_FALSE "no"

/**
 * Creates a literal error expression of type EXPR_TYPE_ERROR. The static type
 * of such an expression is known a priori and identical to the expression's
 * dynamic type. Hence, type-checking a literal expression is not necessary.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param value
 *  The error value of the expression, e.g. the message "Division by zero".
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_literal_t *
expr_literal_create_error(struct var_scope_t *scope, char const *value);

/**
 * Creates a literal string expression of type EXPR_TYPE_STRING. The static
 * type of such an expression is known a priori and identical to the
 * expression's dynamic type. Hence, type-checking a literal expression is not
 * necessary.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param value
 *  The string value of the expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_literal_t *
expr_literal_create_string(struct var_scope_t *scope, char const *value);

/**
 * Creates a literal integer expression of type EXPR_TYPE_INTEGER. The static
 * type of such an expression is known a priori and identical to the
 * expression's dynamic type. Hence, type-checking a literal expression is not
 * necessary.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param value
 *  The integer value of the expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_literal_t *
expr_literal_create_integer(struct var_scope_t *scope, int value);

/**
 * Creates a literal boolean expression of type EXPR_TYPE_BOOLEAN. The static
 * type of such an expression is known a priori and identical to the
 * expression's dynamic type. Hence, type-checking a literal expression is not
 * necessary.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param value
 *  The boolean value of the expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_literal_t *
expr_literal_create_boolean(struct var_scope_t *scope, BOOL value);

/**
 * Extracts a value from a literal expression.
 *
 * @param expr
 *  The literal expression.
 * @param result
 *  Points to an object receiving the value.
 */
void
expr_literal_get_value(struct expr_literal_t const *expr, void *result);

/**
 * Casts a literal expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_literal_to_expr(struct expr_literal_t *expr);

#endif
