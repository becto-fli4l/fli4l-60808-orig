/*****************************************************************************
 *  @file expression/expr_logical_and.c
 *  Functions for processing logical AND expressions.
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
#include <expression/expr_logical_and.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_logical_and"

/**
 * Represents a logical AND expression.
 */
struct expr_logical_and_t {
    /**
     * Common part belonging to every expression. This member must be the first
     * one to allow casting between expr_t and expr_variable_t where necessary.
     */
    struct expr_t expr;
    /**
     * The first argument.
     */
    struct expr_t *arg1;
    /**
     * The second argument.
     */
    struct expr_t *arg2;
};

/**
 * Evaluates a logical AND expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a logical AND
 *  expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the logical AND of the
 *  evaluated arguments.
 */
static struct expr_literal_t *
expr_logical_and_evaluate(struct expr_t const *expr,
                          struct expr_eval_context_t *context)
{
    char *arg_err;
    struct expr_literal_t *result;

    struct expr_logical_and_t const *const andexp
        = (struct expr_logical_and_t const *) expr;

    struct expr_literal_t *arg1_eval = expr_evaluate(andexp->arg1, context);
    BOOL arg1_res;
    if (!expr_get_boolean_value(arg1_eval, &arg1_res, &arg_err)) {
        result = expr_literal_create_error(expr->scope, arg_err);
        free(arg_err);
    }
    else if (arg1_res) {
        struct expr_literal_t *arg2_eval = expr_evaluate(andexp->arg2, context);
        BOOL arg2_res;
        if (!expr_get_boolean_value(arg2_eval, &arg2_res, &arg_err)) {
            result = expr_literal_create_error(expr->scope, arg_err);
            free(arg_err);
        }
        else {
            result = expr_literal_create_boolean(expr->scope, arg2_res);
        }

        expr_destroy(&arg2_eval->expr);
    }
    else {
        result = expr_literal_create_boolean(expr->scope, FALSE);
    }

    expr_destroy(&arg1_eval->expr);
    return result;
}

/**
 * Prints a logical AND expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a logical AND
 *  expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
static char *
expr_logical_and_toString(struct expr_t const *expr)
{
    struct expr_logical_and_t const *const andexp
        = (struct expr_logical_and_t const *) expr;
    char *arg1_str = expr_to_string(andexp->arg1);
    char *arg2_str = expr_to_string(andexp->arg2);
    char *result = safe_sprintf("(%s) && (%s)", arg1_str, arg2_str);
    free(arg1_str);
    free(arg2_str);
    return result;
}

/**
 * Clones a logical AND expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a logical AND
 *  expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_logical_and_clone(struct expr_t const *expr)
{
    struct expr_logical_and_t const *const andexp
        = (struct expr_logical_and_t const *) expr;
    return &expr_logical_and_create(
        expr->scope,
        expr_clone(andexp->arg1),
        expr_clone(andexp->arg2)
    )->expr;
}

/**
 * Compares two logical AND expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a logical AND
 *  expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a logical AND
 *  expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_logical_and_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_logical_and_t const *andexp1
        = (struct expr_logical_and_t const *) expr1;
    struct expr_logical_and_t const *andexp2
        = (struct expr_logical_and_t const *) expr2;
    return expr_equal(andexp1->arg1, andexp2->arg1)
        && expr_equal(andexp1->arg2, andexp2->arg2);
}

/**
 * Frees a logical AND expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a logical AND
 *  expression.
 */
static void
expr_logical_and_free(struct expr_t *expr)
{
    struct expr_logical_and_t const *const andexp
        = (struct expr_logical_and_t const *) expr;
    expr_destroy(andexp->arg1);
    expr_destroy(andexp->arg2);
    expr_common_free(expr);
}

struct expr_logical_and_t *
expr_logical_and_create(
    struct var_scope_t *scope,
    struct expr_t *arg1,
    struct expr_t *arg2
)
{
    struct expr_logical_and_t *const andexp
        = (struct expr_logical_and_t *) safe_malloc(sizeof(struct expr_logical_and_t));

    expr_common_init(
        &andexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_LOGICAL_AND,
        scope,
        &expr_logical_and_evaluate,
        &expr_logical_and_toString,
        &expr_logical_and_clone,
        &expr_logical_and_equal,
        &expr_logical_and_free
    );

    andexp->arg1 = arg1;
    andexp->arg2 = arg2;

    return andexp;
}

struct expr_t *
expr_logical_and_to_expr(struct expr_logical_and_t *expr)
{
    return &expr->expr;
}

struct expr_t *
expr_logical_and_get_left_arg(struct expr_logical_and_t *expr)
{
    return expr->arg1;
}

struct expr_t *
expr_logical_and_get_right_arg(struct expr_logical_and_t *expr)
{
    return expr->arg2;
}
