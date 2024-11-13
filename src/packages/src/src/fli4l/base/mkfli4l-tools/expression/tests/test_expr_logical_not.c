/*****************************************************************************
 *  @file test_expr_logical_not.c
 *  Functions for testing logical NOT expressions.
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

#include <testing/tests.h>
#include <expression.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr_logical_not"

/**
 * Tests expr_kind_to_string().
 */
mu_test_begin(test_expr_logical_not_kindtostring)
    mu_assert_eq_str("logical not", expr_kind_to_string(EXPR_KIND_LOGICAL_NOT));
mu_test_end()

/**
 * Tests expr_get_type().
 */
mu_test_begin(test_expr_logical_not_gettype)
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert_eq_int(EXPR_TYPE_UNKNOWN, expr_get_type(expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert_eq_int(EXPR_TYPE_BOOLEAN, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_kind().
 */
mu_test_begin(test_expr_logical_not_getkind)
    struct expr_t *arg;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert_eq_int(EXPR_KIND_LOGICAL_NOT, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_scope().
 */
mu_test_begin(test_expr_logical_not_getscope)
    struct expr_t *arg;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(expr_get_scope(expr) == var_get_global_scope());
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_logical_not_get_arg().
 */
mu_test_begin(test_expr_logical_not_getarg)
    struct expr_t *arg;
    struct expr_logical_not_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_create(var_get_global_scope(), arg);
    mu_assert(expr_logical_not_get_arg(expr) == arg);
    expr_destroy(expr_logical_not_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of !TRUE.
 */
mu_test_begin(test_expr_logical_not_eval_not_TRUE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert_eq_int(TRUE, expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(FALSE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of !FALSE.
 */
mu_test_begin(test_expr_logical_not_eval_not_FALSE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert_eq_int(TRUE, expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(TRUE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of !42.
 */
mu_test_begin(test_expr_logical_not_eval_not_integer)
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(expr_equal(expr, error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of !"Hallo".
 */
mu_test_begin(test_expr_logical_not_eval_not_string)
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(expr_equal(expr, error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of ![error].
 */
mu_test_begin(test_expr_logical_not_eval_not_error)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert_eq_int(FALSE, expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on !TRUE.
 */
mu_test_begin(test_expr_logical_not_tostring_not_TRUE)
    struct expr_t *arg;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    value = expr_to_string(expr);
    mu_assert_eq_str("!(yes)", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on !FALSE.
 */
mu_test_begin(test_expr_logical_not_tostring_not_FALSE)
    struct expr_t *arg;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    value = expr_to_string(expr);
    mu_assert_eq_str("!(no)", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal logical NOT expressions.
 */
mu_test_begin(test_expr_logical_not_equal1)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    expr2 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    mu_assert_eq_int(TRUE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal logical NOT expressions.
 */
mu_test_begin(test_expr_logical_not_equal2)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    expr2 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    mu_assert_eq_int(FALSE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a logical NOT expression.
 */
mu_test_begin(test_expr_logical_not_clone)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    expr2 = expr_clone(expr1);
    mu_assert_eq_int(TRUE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_logical_not(void)
{
    mu_run_test(test_expr_logical_not_kindtostring);
    mu_run_test(test_expr_logical_not_gettype);
    mu_run_test(test_expr_logical_not_getkind);
    mu_run_test(test_expr_logical_not_getscope);
    mu_run_test(test_expr_logical_not_getarg);
    mu_run_test(test_expr_logical_not_eval_not_TRUE);
    mu_run_test(test_expr_logical_not_eval_not_FALSE);
    mu_run_test(test_expr_logical_not_eval_not_integer);
    mu_run_test(test_expr_logical_not_eval_not_string);
    mu_run_test(test_expr_logical_not_eval_not_error);
    mu_run_test(test_expr_logical_not_tostring_not_TRUE);
    mu_run_test(test_expr_logical_not_tostring_not_FALSE);
    mu_run_test(test_expr_logical_not_equal1);
    mu_run_test(test_expr_logical_not_equal2);
    mu_run_test(test_expr_logical_not_clone);
}
