/*****************************************************************************
 *  @file expression/expr_unequal.c
 *  Functions for processing inequality comparison expressions.
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
#include <expression/expr_unequal.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_unequal"

/**
 * Represents an inequality comparison expression.
 */
struct expr_unequal_t {
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
 * Evaluates an inequality comparison expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is an inequality
 *  comparison expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the result of comparing
 *  the evaluated arguments.
 */
static struct expr_literal_t *
expr_unequal_evaluate(struct expr_t const *expr,
                    struct expr_eval_context_t *context)
{
    struct expr_literal_t *result = NULL;

    struct expr_unequal_t const *const eqexp
        = (struct expr_unequal_t const *) expr;

    struct expr_literal_t *arg1_eval = expr_evaluate(eqexp->arg1, context);
    struct expr_literal_t *arg2_eval = expr_evaluate(eqexp->arg2, context);

    if (arg1_eval->expr.type != arg2_eval->expr.type) {
        if (expr_is_assignable_to(arg1_eval->expr.type, arg2_eval->expr.type)) {
            struct expr_literal_t *arg = expr_cast(arg1_eval, arg2_eval->expr.type);
            expr_destroy(&arg1_eval->expr);
            arg1_eval = arg;
        }
        else {
            struct expr_literal_t *arg;
            my_assert(expr_is_assignable_to(arg2_eval->expr.type, arg1_eval->expr.type));
            arg = expr_cast(arg2_eval, arg1_eval->expr.type);
            expr_destroy(&arg2_eval->expr);
            arg2_eval = arg;
        }
    }

    switch (arg1_eval->expr.type) {
    case EXPR_TYPE_STRING :
        {
            char *val1 = NULL;
            char *val2 = NULL;
            char *arg_err = NULL;
            BOOL b;

            b = expr_get_string_value(arg1_eval, &val1, &arg_err);
            my_assert_var(b);

            if (!expr_get_string_value(arg2_eval, &val2, &arg_err)) {
                result = expr_literal_create_error(expr->scope, arg_err);
            }
            else {
                result = expr_literal_create_boolean(expr->scope, strcmp(val1, val2) != 0);
            }
            free(arg_err);
            free(val1);
            free(val2);
            break;
        }
    case EXPR_TYPE_INTEGER :
        {
            int val1;
            int val2;
            char *arg_err = NULL;
            BOOL b;

            b = expr_get_integer_value(arg1_eval, &val1, &arg_err);
            my_assert_var(b);

            if (!expr_get_integer_value(arg2_eval, &val2, &arg_err)) {
                result = expr_literal_create_error(expr->scope, arg_err);
            }
            else {
                result = expr_literal_create_boolean(expr->scope, val1 != val2);
            }
            free(arg_err);
            break;
        }
    case EXPR_TYPE_BOOLEAN :
        {
            BOOL val1;
            BOOL val2;
            char *arg_err = NULL;
            BOOL b;

            b = expr_get_boolean_value(arg1_eval, &val1, &arg_err);
            my_assert_var(b);

            if (!expr_get_boolean_value(arg2_eval, &val2, &arg_err)) {
                result = expr_literal_create_error(expr->scope, arg_err);
            }
            else {
                result = expr_literal_create_boolean(expr->scope, val1 != val2);
            }
            free(arg_err);
            break;
        }
    case EXPR_TYPE_ERROR :
        result = (struct expr_literal_t *) expr_clone(&arg1_eval->expr);
        break;
    }

    expr_destroy(&arg1_eval->expr);
    expr_destroy(&arg2_eval->expr);
    return result;
}

/**
 * Prints an inequality comparison expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is an inequality
 *  comparison expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
static char *
expr_unequal_toString(struct expr_t const *expr)
{
    struct expr_unequal_t const *const eqexp
        = (struct expr_unequal_t const *) expr;
    char *arg1_str = expr_to_string(eqexp->arg1);
    char *arg2_str = expr_to_string(eqexp->arg2);
    char *result = safe_sprintf("(%s) != (%s)", arg1_str, arg2_str);
    free(arg1_str);
    free(arg2_str);
    return result;
}

/**
 * Clones an inequality comparison expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is an inequality
 *  comparison expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_unequal_clone(struct expr_t const *expr)
{
    struct expr_unequal_t const *const eqexp
        = (struct expr_unequal_t const *) expr;
    return &expr_unequal_create(
        expr->scope,
        expr_clone(eqexp->arg1),
        expr_clone(eqexp->arg2)
    )->expr;
}

/**
 * Compares two inequality comparison expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is an inequality
 *  comparison expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is an inequality
 *  comparison expression.
 * @return
 *  TRUE if both expressions are unequal, FALSE otherwise.
 */
static BOOL
expr_unequal_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_unequal_t const *eqexp1
        = (struct expr_unequal_t const *) expr1;
    struct expr_unequal_t const *eqexp2
        = (struct expr_unequal_t const *) expr2;
    return expr_equal(eqexp1->arg1, eqexp2->arg1)
        && expr_equal(eqexp1->arg2, eqexp2->arg2);
}

/**
 * Frees an inequality comparison expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is an inequality
 *  comparison expression.
 */
static void
expr_unequal_free(struct expr_t *expr)
{
    struct expr_unequal_t const *const eqexp
        = (struct expr_unequal_t const *) expr;
    expr_destroy(eqexp->arg1);
    expr_destroy(eqexp->arg2);
    expr_common_free(expr);
}

struct expr_unequal_t *
expr_unequal_create(
    struct var_scope_t *scope,
    struct expr_t *arg1,
    struct expr_t *arg2
)
{
    struct expr_unequal_t *const eqexp
        = (struct expr_unequal_t *) safe_malloc(sizeof(struct expr_unequal_t));

    expr_common_init(
        &eqexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_UNEQUAL,
        scope,
        &expr_unequal_evaluate,
        &expr_unequal_toString,
        &expr_unequal_clone,
        &expr_unequal_equal,
        &expr_unequal_free
    );

    eqexp->arg1 = arg1;
    eqexp->arg2 = arg2;

    return eqexp;
}

struct expr_t *
expr_unequal_to_expr(struct expr_unequal_t *expr)
{
    return &expr->expr;
}

struct expr_t *
expr_unequal_get_left_arg(struct expr_unequal_t *expr)
{
    return expr->arg1;
}

struct expr_t *
expr_unequal_get_right_arg(struct expr_unequal_t *expr)
{
    return expr->arg2;
}
