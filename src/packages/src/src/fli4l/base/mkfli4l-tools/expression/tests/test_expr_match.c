/*****************************************************************************
 *  @file test_expr_match.c
 *  Functions for testing regular expression matching expressions.
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
#include <var.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr_match"

/**
 * Tests expr_kind_to_string().
 */
mu_test_begin(test_expr_match_kindtostring)
    mu_assert_eq_str("match", expr_kind_to_string(EXPR_KIND_MATCH));
mu_test_end()

/**
 * Tests expr_get_type().
 */
mu_test_begin(test_expr_match_gettype)
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
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
mu_test_begin(test_expr_match_getkind)
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert_eq_int(EXPR_KIND_MATCH, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_scope().
 */
mu_test_begin(test_expr_match_getscope)
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_get_scope(expr) == var_get_global_scope());
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_match_get_left_arg() and expr_match_get_right_arg().
 */
mu_test_begin(test_expr_match_getarg)
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_match_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_create(var_get_global_scope(), arg, regex);
    mu_assert(expr_match_get_left_arg(expr) == arg);
    mu_assert(expr_match_get_right_arg(expr) == regex);
    expr_destroy(expr_match_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a regular expression which is matched (1).
 */
mu_test_begin(test_expr_match_eval_true1)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));

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
 * Tests evaluation with a string and a regular expression which is matched (2).
 */
mu_test_begin(test_expr_match_eval_true2)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abcHallodef"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(value);
    expr_destroy(expr_literal_to_expr(result));

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
 * Tests evaluation with a string and a regular expression which is not
 * matched (1).
 */
mu_test_begin(test_expr_match_eval_false1)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hall"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));

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
 * Tests evaluation with a string and a regular expression which is not
 * matched (2).
 */
mu_test_begin(test_expr_match_eval_false2)
    BOOL value;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallox"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o$"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert(!value);
    expr_destroy(expr_literal_to_expr(result));

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
 * Tests evaluation with a string and a regular expression with subexpressions
 * which is matched (1).
 */
mu_test_begin(test_expr_match_eval_subexp1)
    BOOL match_result;
    char *error_msg;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(match_result);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_N");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_1");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("ll", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a regular expression with subexpressions
 * which is matched (2).
 */
mu_test_begin(test_expr_match_eval_subexp2)
    BOOL match_result;
    char *error_msg;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "ad"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a(bc)*(d)"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(match_result);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_N");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a regular expression with subexpressions
 * which is matched (3).
 */
mu_test_begin(test_expr_match_eval_subexp3)
    BOOL match_result;
    char *error_msg;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "ad"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a(b*)(d)"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(match_result);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_N");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("MATCH_%");
    mu_assert(var);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("d", value->value);
    var_value_destroy(value);

    var_instance_destroy(inst);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a regular expression with subexpressions
 * in a nested scope.
 */
mu_test_begin(test_expr_match_eval_subexp4)
    BOOL match_result;
    char *error_msg;
    struct var_scope_t *scope;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    arg = expr_literal_to_expr(expr_literal_create_string(scope, "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(scope, "Ha(l*)o"));
    expr = expr_match_to_expr(expr_match_create(scope, arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(match_result);

    inst = var_instance_create_from_string(scope, "MATCH_N");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    inst = var_instance_create_from_string(scope, "MATCH_1");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("ll", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a regular expression which references
 * other variable types.
 */
mu_test_begin(test_expr_match_eval_ref)
    BOOL match_result;
    char *error_msg;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a1234b"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")b"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(match_result);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_N");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    var_instance_destroy(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    inst = var_instance_create_from_string(var_get_global_scope(), "MATCH_1");
    mu_assert(inst);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1234", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an error and a regular expression.
 */
mu_test_begin(test_expr_match_eval_error1)
    BOOL match_result;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and an error as regular expression.
 */
mu_test_begin(test_expr_match_eval_error2)
    BOOL match_result;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert_eq_str("Error", error_msg);
    free(error_msg);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and an integer as regular expression.
 */
mu_test_begin(test_expr_match_eval_error3)
    BOOL match_result;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(!match_result);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with a string and a boolean as regular expression.
 */
mu_test_begin(test_expr_match_eval_error4)
    BOOL match_result;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert(!match_result);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an unwriteable MATCH_% array.
 */
mu_test_begin(test_expr_match_eval_error5)
    BOOL match_result;
    char *error_msg;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_properties_t *props;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), ".*"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    props = var_properties_create();
    var = var_add(var_get_global_scope(),
        "MATCH_%",
        vartype_get(VARTYPE_PREDEFINED_ANYTHING),
        VAR_PRIORITY_SCRIPT_TRANSIENT,
        location_create_internal(PACKAGE_FILE_TYPE_VARDEF),
        props
    );
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "XYZ", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));

    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert_eq_str("cannot clear match array 'MATCH_%'", error_msg);
    free(error_msg);

    var_instance_destroy(inst);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation with an invalid regular expression.
 */
mu_test_begin(test_expr_match_eval_error6)
    BOOL match_result;
    char *error_msg;
    struct expr_t *arg;
    struct expr_t *regex;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    arg = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    regex = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "("));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg, regex));
    mu_assert(expr_typecheck_expression(expr, &error_expr));

    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &match_result, &error_msg));
    mu_assert_eq_str("invalid regular expression '('", error_msg);
    free(error_msg);

    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a regular expression matching expression.
 */
mu_test_begin(test_expr_match_tostring)
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    value = expr_to_string(expr);
    mu_assert_eq_str("(\"Hallo\") =~ (\"Ha(l*)o\")", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal regular expression matching expressions.
 */
mu_test_begin(test_expr_match_equal1)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    expr2 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal regular expression matching expressions (1).
 */
mu_test_begin(test_expr_match_equal2)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    expr2 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallox")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    mu_assert(!expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal regular expression matching expressions (2).
 */
mu_test_begin(test_expr_match_equal3)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
    ));
    expr2 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)ox"))
    ));
    mu_assert(!expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a regular expression matching expression.
 */
mu_test_begin(test_expr_match_clone)
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr1 = expr_match_to_expr(expr_match_create(var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Ha(l*)o"))
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
test_expr_match(void)
{
    set_log_level(LOG_EXP);
    mu_run_test(test_expr_match_kindtostring);
    mu_run_test(test_expr_match_gettype);
    mu_run_test(test_expr_match_getkind);
    mu_run_test(test_expr_match_getscope);
    mu_run_test(test_expr_match_getarg);
    mu_run_test(test_expr_match_eval_true1);
    mu_run_test(test_expr_match_eval_true2);
    mu_run_test(test_expr_match_eval_false1);
    mu_run_test(test_expr_match_eval_false2);
    mu_run_test(test_expr_match_eval_subexp1);
    mu_run_test(test_expr_match_eval_subexp2);
    mu_run_test(test_expr_match_eval_subexp3);
    mu_run_test(test_expr_match_eval_subexp4);
    mu_run_test(test_expr_match_eval_ref);
    mu_run_test(test_expr_match_eval_error1);
    mu_run_test(test_expr_match_eval_error2);
    mu_run_test(test_expr_match_eval_error3);
    mu_run_test(test_expr_match_eval_error4);
    mu_run_test(test_expr_match_eval_error5);
    mu_run_test(test_expr_match_eval_error6);
    mu_run_test(test_expr_match_tostring);
    mu_run_test(test_expr_match_equal1);
    mu_run_test(test_expr_match_equal2);
    mu_run_test(test_expr_match_equal3);
    mu_run_test(test_expr_match_clone);
}
