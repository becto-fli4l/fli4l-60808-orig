/*****************************************************************************
 *  @file expression/expr_tobool.c
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

#include <expression/expr_internal.h>
#include <expression/expr_tobool.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_tobool"

/**
 * Represents a TOBOOL expression.
 */
struct expr_tobool_t {
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
 * Evaluates a TOBOOL expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a TOBOOL
 *  expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the conversion of the
 *  evaluated argument to boolean, mapping error expressions to FALSE.
 */
static struct expr_literal_t *
expr_tobool_evaluate(struct expr_t const *expr,
                     struct expr_eval_context_t *context)
{
    BOOL arg_res;
    char *arg_err;
    struct expr_literal_t *result;

    struct expr_tobool_t const *const toboolexp
        = (struct expr_tobool_t const *) expr;
    struct expr_literal_t *arg_eval = expr_evaluate(toboolexp->arg, context);

    if (!expr_get_boolean_value(arg_eval, &arg_res, &arg_err)) {
        result = expr_literal_create_boolean(expr->scope, FALSE);
        free(arg_err);
    }
    else {
        result = expr_literal_create_boolean(expr->scope, arg_res);
    }

    expr_destroy(&arg_eval->expr);
    return result;
}

/**
 * Prints a TOBOOL expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a TOBOOL
 *  expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
static char *
expr_tobool_toString(struct expr_t const *expr)
{
    struct expr_tobool_t const *const toboolexp
        = (struct expr_tobool_t const *) expr;
    char *arg_str = expr_to_string(toboolexp->arg);
    char *result = safe_sprintf("[%s]", arg_str);
    free(arg_str);
    return result;
}

/**
 * Clones a TOBOOL expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a TOBOOL
 *  expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_tobool_clone(struct expr_t const *expr)
{
    struct expr_tobool_t const *const toboolexp
        = (struct expr_tobool_t const *) expr;
    return &expr_tobool_create(expr->scope, expr_clone(toboolexp->arg))->expr;
}

/**
 * Frees a TOBOOL expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a TOBOOL
 *  expression.
 */
static void
expr_tobool_free(struct expr_t *expr)
{
    struct expr_tobool_t const *const toboolexp
        = (struct expr_tobool_t const *) expr;
    expr_destroy(toboolexp->arg);
    expr_common_free(expr);
}

/**
 * Compares two TOBOOL expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a TOBOOL
 *  expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a TOBOOL
 *  expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_tobool_equal(struct expr_t const *expr1,
                          struct expr_t const *expr2)
{
    struct expr_tobool_t const *toboolexp1
        = (struct expr_tobool_t const *) expr1;
    struct expr_tobool_t const *toboolexp2
        = (struct expr_tobool_t const *) expr2;
    return expr_equal(toboolexp1->arg, toboolexp2->arg);
}

struct expr_tobool_t *
expr_tobool_create(struct var_scope_t *scope, struct expr_t *arg)
{
    struct expr_tobool_t *const toboolexp
        = (struct expr_tobool_t *)
            safe_malloc(sizeof(struct expr_tobool_t));

    expr_common_init(
        &toboolexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_TOBOOL,
        scope,
        &expr_tobool_evaluate,
        &expr_tobool_toString,
        &expr_tobool_clone,
        &expr_tobool_equal,
        &expr_tobool_free
    );

    toboolexp->arg = arg;

    return toboolexp;
}

struct expr_t *
expr_tobool_to_expr(struct expr_tobool_t *expr)
{
    return &expr->expr;
}

struct expr_t *
expr_tobool_get_arg(struct expr_tobool_t *expr)
{
    return expr->arg;
}
