/*****************************************************************************
 *  @file test_expr_logical_or.c
 *  Functions for testing logical OR expressions.
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

#define MU_TESTSUITE "expression.expr_logical_or"

/**
 * Tests expr_kind_to_string().
 */
mu_test_begin(test_expr_logical_or_kindtostring)
    mu_assert_eq_str("logical or", expr_kind_to_string(EXPR_KIND_LOGICAL_OR));
mu_test_end()

/**
 * Tests expr_get_type().
 */
mu_test_begin(test_expr_logical_or_gettype)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
mu_test_begin(test_expr_logical_or_getkind)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert_eq_int(EXPR_KIND_LOGICAL_OR, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_scope().
 */
mu_test_begin(test_expr_logical_or_getscope)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_get_scope(expr) == var_get_global_scope());
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_logical_or_get_left_arg() and expr_logical_or_get_right_arg().
 */
mu_test_begin(test_expr_logical_or_getarg)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_logical_or_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_create(var_get_global_scope(), arg1, arg2);
    mu_assert(expr_logical_or_get_left_arg(expr) == arg1);
    mu_assert(expr_logical_or_get_right_arg(expr) == arg2);
    expr_destroy(expr_logical_or_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of TRUE || TRUE.
 */
mu_test_begin(test_expr_logical_or_eval_TRUE_TRUE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of TRUE || FALSE.
 */
mu_test_begin(test_expr_logical_or_eval_TRUE_FALSE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of FALSE || TRUE.
 */
mu_test_begin(test_expr_logical_or_eval_FALSE_TRUE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of FALSE || FALSE.
 */
mu_test_begin(test_expr_logical_or_eval_FALSE_FALSE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of TRUE || [error].
 */
mu_test_begin(test_expr_logical_or_eval_TRUE_error)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of FALSE || [error].
 */
mu_test_begin(test_expr_logical_or_eval_FALSE_error)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of [error] || TRUE.
 */
mu_test_begin(test_expr_logical_or_eval_error_TRUE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of [error] || FALSE.
 */
mu_test_begin(test_expr_logical_or_eval_error_FALSE)
    BOOL value;
    char *error_msg;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
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
 * Tests evaluation of TRUE || 42.
 */
mu_test_begin(test_expr_logical_or_eval_TRUE_integer)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of FALSE || 42.
 */
mu_test_begin(test_expr_logical_or_eval_FALSE_integer)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of 42 || TRUE.
 */
mu_test_begin(test_expr_logical_or_eval_integer_TRUE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of 42 || FALSE.
 */
mu_test_begin(test_expr_logical_or_eval_integer_FALSE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of TRUE || "Hallo".
 */
mu_test_begin(test_expr_logical_or_eval_TRUE_string)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of FALSE || "Hallo".
 */
mu_test_begin(test_expr_logical_or_eval_FALSE_string)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of "Hallo" || TRUE.
 */
mu_test_begin(test_expr_logical_or_eval_string_TRUE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of "Hallo" || FALSE.
 */
mu_test_begin(test_expr_logical_or_eval_string_FALSE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on TRUE || FALSE.
 */
mu_test_begin(test_expr_logical_or_tostring_TRUE_FALSE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    value = expr_to_string(expr);
    mu_assert_eq_str("(yes) || (no)", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on FALSE || TRUE.
 */
mu_test_begin(test_expr_logical_or_tostring_FALSE_TRUE)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    value = expr_to_string(expr);
    mu_assert_eq_str("(no) || (yes)", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal logical OR expressions.
 */
mu_test_begin(test_expr_logical_or_equal1)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    mu_assert_eq_int(TRUE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal logical OR expressions (1).
 */
mu_test_begin(test_expr_logical_or_equal2)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    mu_assert_eq_int(FALSE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal logical OR expressions (2).
 */
mu_test_begin(test_expr_logical_or_equal3)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    mu_assert_eq_int(FALSE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a logical OR expression.
 */
mu_test_begin(test_expr_logical_or_clone)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
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
test_expr_logical_or(void)
{
    mu_run_test(test_expr_logical_or_kindtostring);
    mu_run_test(test_expr_logical_or_gettype);
    mu_run_test(test_expr_logical_or_getkind);
    mu_run_test(test_expr_logical_or_getscope);
    mu_run_test(test_expr_logical_or_getarg);
    mu_run_test(test_expr_logical_or_eval_TRUE_TRUE);
    mu_run_test(test_expr_logical_or_eval_TRUE_FALSE);
    mu_run_test(test_expr_logical_or_eval_FALSE_TRUE);
    mu_run_test(test_expr_logical_or_eval_FALSE_FALSE);
    mu_run_test(test_expr_logical_or_eval_TRUE_error);
    mu_run_test(test_expr_logical_or_eval_FALSE_error);
    mu_run_test(test_expr_logical_or_eval_error_TRUE);
    mu_run_test(test_expr_logical_or_eval_error_FALSE);
    mu_run_test(test_expr_logical_or_eval_TRUE_integer);
    mu_run_test(test_expr_logical_or_eval_FALSE_integer);
    mu_run_test(test_expr_logical_or_eval_integer_TRUE);
    mu_run_test(test_expr_logical_or_eval_integer_FALSE);
    mu_run_test(test_expr_logical_or_eval_TRUE_string);
    mu_run_test(test_expr_logical_or_eval_FALSE_string);
    mu_run_test(test_expr_logical_or_eval_string_TRUE);
    mu_run_test(test_expr_logical_or_eval_string_FALSE);
    mu_run_test(test_expr_logical_or_tostring_TRUE_FALSE);
    mu_run_test(test_expr_logical_or_tostring_FALSE_TRUE);
    mu_run_test(test_expr_logical_or_equal1);
    mu_run_test(test_expr_logical_or_equal2);
    mu_run_test(test_expr_logical_or_equal3);
    mu_run_test(test_expr_logical_or_clone);
}
