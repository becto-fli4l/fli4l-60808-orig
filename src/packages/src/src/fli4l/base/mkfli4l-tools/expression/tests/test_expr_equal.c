/*****************************************************************************
 *  @file test_expr_equal.c
 *  Functions for testing equality comparison expressions.
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

#define MU_TESTSUITE "expression.expr_equal"

/**
 * Tests expr_kind_to_string().
 */
mu_test_begin(test_expr_equal_kindtostring)
    mu_assert_eq_str("equal", expr_kind_to_string(EXPR_KIND_EQUAL));
mu_test_end()

/**
 * Tests expr_get_type().
 */
mu_test_begin(test_expr_equal_gettype)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
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
mu_test_begin(test_expr_equal_getkind)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert_eq_int(EXPR_KIND_EQUAL, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_scope().
 */
mu_test_begin(test_expr_equal_getscope)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_get_scope(expr) == var_get_global_scope());
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_equal_get_left_arg() and expr_equal_get_right_arg().
 */
mu_test_begin(test_expr_equal_getarg)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_equal_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    expr = expr_equal_create(var_get_global_scope(), arg1, arg2);
    mu_assert(expr_equal_get_left_arg(expr) == arg1);
    mu_assert(expr_equal_get_right_arg(expr) == arg2);
    expr_destroy(expr_equal_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two equal strings.
 */
mu_test_begin(test_expr_equal_eval_string_string_eq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two unequal strings.
 */
mu_test_begin(test_expr_equal_eval_string_string_noteq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Welt"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two equal integers.
 */
mu_test_begin(test_expr_equal_eval_integer_integer_eq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two unequal integers.
 */
mu_test_begin(test_expr_equal_eval_integer_integer_noteq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two equal boolean values.
 */
mu_test_begin(test_expr_equal_eval_boolean_boolean_eq)
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
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with two unequal boolean values.
 */
mu_test_begin(test_expr_equal_eval_boolean_boolean_noteq)
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
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and an integer which both have equal string
 * representations.
 */
mu_test_begin(test_expr_equal_eval_string_integer_eq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "42"));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and an integer which don't have equal string
 * representations.
 */
mu_test_begin(test_expr_equal_eval_string_integer_noteq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "042"));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an integer and a string which both have equal string
 * representations.
 */
mu_test_begin(test_expr_equal_eval_integer_string_eq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "42"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an integer and a string which don't have equal string
 * representations.
 */
mu_test_begin(test_expr_equal_eval_integer_string_noteq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "042"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a boolean value which both have equal
 * string representations.
 */
mu_test_begin(test_expr_equal_eval_string_boolean_eq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "yes"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a boolean value which don't have equal
 * string representations.
 */
mu_test_begin(test_expr_equal_eval_string_boolean_noteq)
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

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "TRUE"));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a boolean value and a string which both have equal
 * string representations.
 */
mu_test_begin(test_expr_equal_eval_boolean_string_eq)
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
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "yes"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a boolean value and a string which don't have equal
 * string representations.
 */
mu_test_begin(test_expr_equal_eval_boolean_string_noteq)
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
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "TRUE"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an integer and a boolean value.
 */
mu_test_begin(test_expr_equal_eval_integer_boolean)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 1));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a boolean value and an integer.
 */
mu_test_begin(test_expr_equal_eval_boolean_integer)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an error and a string.
 */
mu_test_begin(test_expr_equal_eval_error_string)
    BOOL value;
    char *error_msg = NULL;
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
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and an error.
 */
mu_test_begin(test_expr_equal_eval_string_error)
    BOOL value;
    char *error_msg = NULL;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Welt"));
    arg2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an error and an integer.
 */
mu_test_begin(test_expr_equal_eval_error_integer)
    BOOL value;
    char *error_msg = NULL;
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
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an integer and an error.
 */
mu_test_begin(test_expr_equal_eval_integer_error)
    BOOL value;
    char *error_msg = NULL;
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23));
    arg2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an error and a boolean value.
 */
mu_test_begin(test_expr_equal_eval_error_boolean)
    BOOL value;
    char *error_msg = NULL;
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
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a boolean value and an error.
 */
mu_test_begin(test_expr_equal_eval_boolean_error)
    BOOL value;
    char *error_msg = NULL;
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
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on an equality comparison expression.
 */
mu_test_begin(test_expr_equal_tostring)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    value = expr_to_string(expr);
    mu_assert_eq_str("(yes) == (42)", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal equality comparison expressions.
 */
mu_test_begin(test_expr_equal_equal1)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal equality comparison expressions (1).
 */
mu_test_begin(test_expr_equal_equal2)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    mu_assert(!expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal equality comparison expressions (2).
 */
mu_test_begin(test_expr_equal_equal3)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    mu_assert(!expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on an equality comparison expression.
 */
mu_test_begin(test_expr_equal_clone)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    expr2 = expr_clone(expr1);
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_equal(void)
{
    mu_run_test(test_expr_equal_kindtostring);
    mu_run_test(test_expr_equal_gettype);
    mu_run_test(test_expr_equal_getkind);
    mu_run_test(test_expr_equal_getscope);
    mu_run_test(test_expr_equal_getarg);
    mu_run_test(test_expr_equal_eval_string_string_eq);
    mu_run_test(test_expr_equal_eval_string_string_noteq);
    mu_run_test(test_expr_equal_eval_integer_integer_eq);
    mu_run_test(test_expr_equal_eval_integer_integer_noteq);
    mu_run_test(test_expr_equal_eval_boolean_boolean_eq);
    mu_run_test(test_expr_equal_eval_boolean_boolean_noteq);
    mu_run_test(test_expr_equal_eval_string_integer_eq);
    mu_run_test(test_expr_equal_eval_string_integer_noteq);
    mu_run_test(test_expr_equal_eval_integer_string_eq);
    mu_run_test(test_expr_equal_eval_integer_string_noteq);
    mu_run_test(test_expr_equal_eval_string_boolean_eq);
    mu_run_test(test_expr_equal_eval_string_boolean_noteq);
    mu_run_test(test_expr_equal_eval_boolean_string_eq);
    mu_run_test(test_expr_equal_eval_boolean_string_noteq);
    mu_run_test(test_expr_equal_eval_integer_boolean);
    mu_run_test(test_expr_equal_eval_boolean_integer);
    mu_run_test(test_expr_equal_eval_error_string);
    mu_run_test(test_expr_equal_eval_string_error);
    mu_run_test(test_expr_equal_eval_error_integer);
    mu_run_test(test_expr_equal_eval_integer_error);
    mu_run_test(test_expr_equal_eval_error_boolean);
    mu_run_test(test_expr_equal_eval_boolean_error);
    mu_run_test(test_expr_equal_tostring);
    mu_run_test(test_expr_equal_equal1);
    mu_run_test(test_expr_equal_equal2);
    mu_run_test(test_expr_equal_equal3);
    mu_run_test(test_expr_equal_clone);
}
