/*****************************************************************************
 *  @file expression/expr_logical_not.c
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

#include <expression/expr_internal.h>
#include <expression/expr_logical_not.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_logical_not"

/**
 * Represents a logical NOT expression.
 */
struct expr_logical_not_t {
    /**
     * Common part belonging to every expression. This member must be the first
     * one to allow casting between expr_t and expr_variable_t where necessary.
     */
    struct expr_t expr;
    /**
     * The underlying expression.
     */
    struct expr_t *arg;
};

/**
 * Evaluates a logical NOT expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a logical NOT
 *  expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the negation of the
 *  evaluated argument.
 */
static struct expr_literal_t *
expr_logical_not_evaluate(struct expr_t const *expr,
                          struct expr_eval_context_t *context)
{
    BOOL arg_res;
    char *arg_err;
    struct expr_literal_t *result;

    struct expr_logical_not_t const *const notexp
        = (struct expr_logical_not_t const *) expr;
    struct expr_literal_t *arg_eval = expr_evaluate(notexp->arg, context);

    if (!expr_get_boolean_value(arg_eval, &arg_res, &arg_err)) {
        result = expr_literal_create_error(expr->scope, arg_err);
        free(arg_err);
    }
    else {
        result = expr_literal_create_boolean(expr->scope, !arg_res);
    }

    expr_destroy(&arg_eval->expr);
    return result;
}

/**
 * Prints a logical NOT expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a logical NOT
 *  expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
static char *
expr_logical_not_toString(struct expr_t const *expr)
{
    struct expr_logical_not_t const *const notexp
        = (struct expr_logical_not_t const *) expr;
    char *arg_str = expr_to_string(notexp->arg);
    char *result = safe_sprintf("!(%s)", arg_str);
    free(arg_str);
    return result;
}

/**
 * Clones a logical NOT expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a logical NOT
 *  expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_logical_not_clone(struct expr_t const *expr)
{
    struct expr_logical_not_t const *const notexp
        = (struct expr_logical_not_t const *) expr;
    return &expr_logical_not_create(expr->scope, expr_clone(notexp->arg))->expr;
}

/**
 * Frees a logical NOT expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a logical NOT
 *  expression.
 */
static void
expr_logical_not_free(struct expr_t *expr)
{
    struct expr_logical_not_t const *const notexp
        = (struct expr_logical_not_t const *) expr;
    expr_destroy(notexp->arg);
    expr_common_free(expr);
}

/**
 * Compares two logical NOT expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a logical NOT
 *  expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a logical NOT
 *  expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_logical_not_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_logical_not_t const *notexp1
        = (struct expr_logical_not_t const *) expr1;
    struct expr_logical_not_t const *notexp2
        = (struct expr_logical_not_t const *) expr2;
    return expr_equal(notexp1->arg, notexp2->arg);
}

struct expr_logical_not_t *
expr_logical_not_create(struct var_scope_t *scope, struct expr_t *arg)
{
    struct expr_logical_not_t *const notexp
        = (struct expr_logical_not_t *) safe_malloc(sizeof(struct expr_logical_not_t));

    expr_common_init(
        &notexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_LOGICAL_NOT,
        scope,
        &expr_logical_not_evaluate,
        &expr_logical_not_toString,
        &expr_logical_not_clone,
        &expr_logical_not_equal,
        &expr_logical_not_free
    );

    notexp->arg = arg;

    return notexp;
}

struct expr_t *
expr_logical_not_to_expr(struct expr_logical_not_t *expr)
{
    return &expr->expr;
}

struct expr_t *
expr_logical_not_get_arg(struct expr_logical_not_t *expr)
{
    return expr->arg;
}
