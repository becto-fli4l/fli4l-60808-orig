/*****************************************************************************
 *  @file test_expr_literal_error.c
 *  Functions for testing literal error expressions.
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

#define MU_TESTSUITE "expression.expr_literal"

/**
 * Tests expr_get_type().
 */
mu_test_begin(test_expr_literal_error_gettype)
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Hallo"));
    mu_assert_eq_int(EXPR_TYPE_ERROR, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_kind().
 */
mu_test_begin(test_expr_literal_error_getkind)
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Hallo"));
    mu_assert_eq_int(EXPR_KIND_LITERAL, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_scope().
 */
mu_test_begin(test_expr_literal_error_getscope)
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Hallo"));
    mu_assert(expr_get_scope(expr) == var_get_global_scope());
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the evaluation of a literal error expression.
 */
mu_test_begin(test_expr_literal_error_eval)
    char *value;
    char *error_msg;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *e;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Hallo"));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    e = expr_evaluate(expr, NULL);
    mu_assert_eq_int(expr_get_type(expr), expr_get_type(expr_literal_to_expr(e)));
    mu_assert_eq_int(expr_get_kind(expr), expr_get_kind(expr_literal_to_expr(e)));
    mu_assert_eq_int(FALSE, expr_get_string_value(e, &value, &error_msg));
    mu_assert_eq_str("Hallo", error_msg);

    free(error_msg);
    expr_destroy(expr_literal_to_expr(e));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a literal error expression.
 */
mu_test_begin(test_expr_literal_error_tostring)
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Hallo"));
    value = expr_to_string(expr);
    mu_assert_eq_str("Hallo", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal error expressions.
 */
mu_test_begin(test_expr_literal_error_equal1)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    mu_assert_eq_int(TRUE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal error expressions.
 */
mu_test_begin(test_expr_literal_error_equal2)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error 1"));
    expr2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error 2"));
    mu_assert_eq_int(FALSE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on an error expression.
 */
mu_test_begin(test_expr_literal_error_clone)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr2 = expr_clone(expr1);
    mu_assert_eq_int(TRUE, expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_literal_error(void)
{
    mu_run_test(test_expr_literal_error_gettype);
    mu_run_test(test_expr_literal_error_getkind);
    mu_run_test(test_expr_literal_error_getscope);
    mu_run_test(test_expr_literal_error_eval);
    mu_run_test(test_expr_literal_error_tostring);
    mu_run_test(test_expr_literal_error_equal1);
    mu_run_test(test_expr_literal_error_equal2);
    mu_run_test(test_expr_literal_error_clone);
}
