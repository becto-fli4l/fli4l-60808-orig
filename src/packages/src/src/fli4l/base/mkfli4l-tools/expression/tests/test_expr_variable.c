/*****************************************************************************
 *  @file test_expr_variable.c
 *  Functions for testing variable expressions.
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
#include <vartype.h>
#include <var.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr_variable"

/**
 * Tests expr_kind_to_string().
 */
mu_test_begin(test_expr_variable_kindtostring)
    mu_assert_eq_str("variable", expr_kind_to_string(EXPR_KIND_VARIABLE));
mu_test_end()

/**
 * Tests expr_get_type() on a string variable (1).
 */
mu_test_begin(test_expr_variable_gettype_string1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    char const *error_var;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert_eq_int(EXPR_TYPE_UNKNOWN, expr_get_type(expr));
    mu_assert(expr_resolve_variable_expressions(expr, &error_var));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert_eq_int(EXPR_TYPE_STRING, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_type() on a string variable (2).
 */
mu_test_begin(test_expr_variable_gettype_string2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert_eq_int(EXPR_TYPE_STRING, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_type() on an integer variable (1).
 */
mu_test_begin(test_expr_variable_gettype_integer1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    char const *error_var;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert_eq_int(EXPR_TYPE_UNKNOWN, expr_get_type(expr));
    mu_assert(expr_resolve_variable_expressions(expr, &error_var));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert_eq_int(EXPR_TYPE_INTEGER, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_type() on an integer variable (2).
 */
mu_test_begin(test_expr_variable_gettype_integer2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert_eq_int(EXPR_TYPE_INTEGER, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_type() on a boolean variable (1).
 */
mu_test_begin(test_expr_variable_gettype_boolean1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    char const *error_var;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert_eq_int(EXPR_TYPE_UNKNOWN, expr_get_type(expr));
    mu_assert(expr_resolve_variable_expressions(expr, &error_var));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert_eq_int(EXPR_TYPE_BOOLEAN, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_type() on a boolean variable (2).
 */
mu_test_begin(test_expr_variable_gettype_boolean2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert_eq_int(EXPR_TYPE_BOOLEAN, expr_get_type(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_kind() (1).
 */
mu_test_begin(test_expr_variable_getkind1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert_eq_int(EXPR_KIND_VARIABLE, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_get_kind() (2).
 */
mu_test_begin(test_expr_variable_getkind2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert_eq_int(EXPR_KIND_VARIABLE, expr_get_kind(expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_variable_get_state() and expr_variable_get_unresolved_name() on a
 * variable expression in state UNRESOLVED.
 */
mu_test_begin(test_expr_variable_get_unresolved_name)
    struct expr_variable_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_variable_create_by_name(var_get_global_scope(), "ABC");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(expr));
    mu_assert_eq_str("ABC", expr_variable_get_unresolved_name(expr));
    expr_destroy(expr_variable_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests expr_variable_get_state() and expr_variable_get_resolved_instance() on
 * a variable expression in state RESOLVED.
 */
mu_test_begin(test_expr_variable_get_resolved_instance)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_variable_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_create(var_get_global_scope(), inst);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(expr));
    mu_assert(inst == expr_variable_get_resolved_instance(expr));
    expr_destroy(expr_variable_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type string (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_string1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type string (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_string2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    char *value = NULL;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("2", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type integer (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_integer1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type integer (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_integer2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    int value = 0;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_integer_value(result, &value, &error_msg));
    mu_assert_eq_int(2, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type integer with invalid contents (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_integer_invalid1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("NUMEXT", "0|[1-9][0-9]*|NaN", "number expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 1));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "NaN", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type integer with invalid contents (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_integer_invalid2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    int value = 0;
    char *error_msg = NULL;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("NUMEXT", "0|[1-9][0-9]*|NaN", "number expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 1));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "NaN", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_integer_value(result, &value, &error_msg));
    mu_assert_eq_str("converting value 'NaN' of variable 'V' to type integer failed", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type boolean (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_boolean1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type boolean (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_boolean2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value = FALSE;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
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
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type boolean with invalid contents (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_boolean_invalid1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "maybe", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * of type boolean with invalid contents (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_boolean_invalid2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value = 0;
    char *error_msg = NULL;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "maybe", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_str("converting value 'maybe' of variable 'V' to type boolean failed", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * without a value (1).
 */
mu_test_begin(test_expr_variable_eval_non_array_empty1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing a non-array variable
 * without a value (2).
 */
mu_test_begin(test_expr_variable_eval_non_array_empty2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    char *value;
    char *error_msg = NULL;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("reading variable 'V' failed", error_msg);
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable (1).
 */
mu_test_begin(test_expr_variable_eval_array1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable (2).
 */
mu_test_begin(test_expr_variable_eval_array2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_instance_t *inst_expr;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    char *value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 0;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("1", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("3", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 1;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("2", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("0", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable
 * providing a sufficient context (1).
 */
mu_test_begin(test_expr_variable_eval_array_context1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_eval_context_t *context;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    context = expr_eval_context_create();
    context->var_inst = var_instance_create(var, 1, NULL);
    context->var_inst->indices[0] = 0;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL)));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, context);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(FALSE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_instance_destroy(context->var_inst);
    expr_eval_context_destroy(context);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable
 * providing a sufficient context (2).
 */
mu_test_begin(test_expr_variable_eval_array_context2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_eval_context_t *context;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    context = expr_eval_context_create();
    context->var_inst = var_instance_create(var, 2, NULL);
    context->var_inst->indices[0] = 0;
    context->var_inst->indices[1] = 123;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL)));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, context);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(FALSE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_instance_destroy(context->var_inst);
    expr_eval_context_destroy(context);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable
 * providing an insufficient context (1).
 */
mu_test_begin(test_expr_variable_eval_array_context3)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_eval_context_t *context;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    context = expr_eval_context_create();
    context->var_inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL)));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, context);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(TRUE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    var_instance_destroy(context->var_inst);
    expr_eval_context_destroy(context);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable
 * providing an insufficient context (2).
 */
mu_test_begin(test_expr_variable_eval_array_context4)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_eval_context_t *context;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    context = expr_eval_context_create();
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL)));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, context);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(TRUE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);
    expr_eval_context_destroy(context);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable
 * providing an insufficient context (3).
 */
mu_test_begin(test_expr_variable_eval_array_context5)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    BOOL value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL)));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_boolean_value(result, &value, &error_msg));
    mu_assert_eq_int(TRUE, value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable with
 * a condition referencing another array variable. Both arrays share the same
 * array bound (1).
 */
mu_test_begin(test_expr_variable_eval_array_with_cond_shared1)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_properties_t *var_props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    var_props = var_properties_create();
    var_properties_set_condition(var_props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable with
 * a condition referencing another array variable. Both arrays share the same
 * array bound (2).
 */
mu_test_begin(test_expr_variable_eval_array_with_cond_shared2)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_properties_t *var_props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_instance_t *inst_expr;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    char *value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    var_props = var_properties_create();
    var_properties_set_condition(var_props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 0;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("1", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("3", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 1;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_string_value(result, &value, &error_msg));
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_string_value(result, &value, &error_msg));
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 2;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("3", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("0", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable with
 * a condition referencing another array variable. The arrays do not share the
 * same array bound (1).
 */
mu_test_begin(test_expr_variable_eval_array_with_cond_unshared1)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_properties_t *var_props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_COND_%", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    var_props = var_properties_create();
    var_properties_set_condition(var_props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests evaluation of a variable expression referencing an array variable with
 * a condition referencing another array variable. The arrays do not share the
 * same array bound (2).
 */
mu_test_begin(test_expr_variable_eval_array_with_cond_unshared2)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_properties_t *var_props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_instance_t *inst_expr;
    struct expr_t *expr;
    struct expr_t *error_expr;
    struct expr_literal_t *result;
    char *value;
    char *error_msg;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_COND_%", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    var_props = var_properties_create();
    var_properties_set_condition(var_props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 0;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("1", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("3", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 1;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_string_value(result, &value, &error_msg));
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(!expr_get_string_value(result, &value, &error_msg));
    free(error_msg);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    inst_expr = var_instance_create(var, 1, NULL);
    inst_expr->indices[0] = 2;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_expr));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("3", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    result = expr_evaluate(expr, NULL);
    mu_assert(expr_get_string_value(result, &value, &error_msg));
    mu_assert_eq_str("0", value);
    free(value);
    expr_destroy(expr_literal_to_expr(result));
    expr_destroy(expr);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a variable expression referencing a non-array variable.
 */
mu_test_begin(test_expr_variable_tostring_non_array)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    value = expr_to_string(expr);
    mu_assert_eq_str(var_get_name(var), value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a variable expression referencing an array variable.
 */
mu_test_begin(test_expr_variable_tostring_array)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    value = expr_to_string(expr);
    mu_assert_eq_str("V_1", value);
    free(value);
    expr_destroy(expr);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 1;
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    value = expr_to_string(expr);
    mu_assert_eq_str("V_2", value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a variable expression referencing a non-array variable
 * by name.
 */
mu_test_begin(test_expr_variable_tostring_non_array_by_name)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    value = expr_to_string(expr);
    mu_assert_eq_str(var_get_name(var), value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests toString() on a variable expression referencing an array variable by
 * name.
 */
mu_test_begin(test_expr_variable_tostring_array_by_name)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    char *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V_%"));
    value = expr_to_string(expr);
    mu_assert_eq_str(var_get_name(var), value);
    free(value);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal variable expressions by name.
 */
mu_test_begin(test_expr_variable_equal1a)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    expr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(expr_equal(expr1, expr2));
    mu_assert(expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two equal variable expressions.
 */
mu_test_begin(test_expr_variable_equal1b)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    inst = var_instance_create(var, 0, NULL);
    expr2 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_equal(expr1, expr2));
    mu_assert(expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal variable expressions that differ in the
 * variable being referenced.
 */
mu_test_begin(test_expr_variable_equal2a)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    var2 = var_add(var_get_global_scope(), "W", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());

    expr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var1)));
    expr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var2)));
    mu_assert(!expr_equal(expr1, expr2));
    mu_assert(!expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal variable expressions that differ in the
 * variable being referenced.
 */
mu_test_begin(test_expr_variable_equal2b)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *inst;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    var2 = var_add(var_get_global_scope(), "W", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());

    inst = var_instance_create(var1, 0, NULL);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    inst = var_instance_create(var2, 0, NULL);
    expr2 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(!expr_equal(expr1, expr2));
    mu_assert(!expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal variable expressions that differ in the indices
 * used (1).
 */
mu_test_begin(test_expr_variable_equal3a)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[1];
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    indices[0] = 0;
    inst = var_instance_create(var, 1, indices);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    indices[0] = 1;
    inst = var_instance_create(var, 1, indices);
    expr2 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(!expr_equal(expr1, expr2));
    mu_assert(!expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal variable expressions that differ in the indices
 * used (2).
 */
mu_test_begin(test_expr_variable_equal3b)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[2];
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    indices[0] = 0;
    indices[1] = 1;
    inst = var_instance_create(var, 1, indices);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    indices[0] = 0;
    indices[1] = 1;
    inst = var_instance_create(var, 2, indices);
    expr2 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(!expr_equal(expr1, expr2));
    mu_assert(!expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests equal() on two unequal variable expressions that differ in state.
 */
mu_test_begin(test_expr_variable_equal4)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    expr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    mu_assert(!expr_equal(expr1, expr2));
    mu_assert(!expr_equal(expr2, expr1));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a variable expression referencing a non-array variable by
 * name.
 */
mu_test_begin(test_expr_variable_clone1a)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    expr2 = expr_clone(expr1);
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a variable expression referencing a non-array variable.
 */
mu_test_begin(test_expr_variable_clone1b)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    expr2 = expr_clone(expr1);
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a variable expression referencing an array variable by name.
 */
mu_test_begin(test_expr_variable_clone2a)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var)));
    expr2 = expr_clone(expr1);
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clone() on a variable expression referencing an array variable.
 */
mu_test_begin(test_expr_variable_clone2b)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[1];
    struct expr_t *expr1;
    struct expr_t *expr2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    indices[0] = 1;
    inst = var_instance_create(var, 1, indices);
    expr1 = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    expr2 = expr_clone(expr1);
    mu_assert(expr_equal(expr1, expr2));
    expr_destroy(expr1);
    expr_destroy(expr2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_variable(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_expr_variable_kindtostring);
    mu_run_test(test_expr_variable_gettype_string1);
    mu_run_test(test_expr_variable_gettype_string2);
    mu_run_test(test_expr_variable_gettype_integer1);
    mu_run_test(test_expr_variable_gettype_integer2);
    mu_run_test(test_expr_variable_gettype_boolean1);
    mu_run_test(test_expr_variable_gettype_boolean2);
    mu_run_test(test_expr_variable_getkind1);
    mu_run_test(test_expr_variable_getkind2);
    mu_run_test(test_expr_variable_get_unresolved_name);
    mu_run_test(test_expr_variable_get_resolved_instance);
    mu_run_test(test_expr_variable_eval_non_array_string1);
    mu_run_test(test_expr_variable_eval_non_array_string2);
    mu_run_test(test_expr_variable_eval_non_array_integer1);
    mu_run_test(test_expr_variable_eval_non_array_integer2);
    mu_run_test(test_expr_variable_eval_non_array_integer_invalid1);
    mu_run_test(test_expr_variable_eval_non_array_integer_invalid2);
    mu_run_test(test_expr_variable_eval_non_array_boolean1);
    mu_run_test(test_expr_variable_eval_non_array_boolean2);
    mu_run_test(test_expr_variable_eval_non_array_boolean_invalid1);
    mu_run_test(test_expr_variable_eval_non_array_boolean_invalid2);
    mu_run_test(test_expr_variable_eval_non_array_empty1);
    mu_run_test(test_expr_variable_eval_non_array_empty2);
    mu_run_test(test_expr_variable_eval_array1);
    mu_run_test(test_expr_variable_eval_array2);
    mu_run_test(test_expr_variable_eval_array_context1);
    mu_run_test(test_expr_variable_eval_array_context2);
    mu_run_test(test_expr_variable_eval_array_context3);
    mu_run_test(test_expr_variable_eval_array_context4);
    mu_run_test(test_expr_variable_eval_array_context5);
    mu_run_test(test_expr_variable_eval_array_with_cond_shared1);
    mu_run_test(test_expr_variable_eval_array_with_cond_shared2);
    mu_run_test(test_expr_variable_eval_array_with_cond_unshared1);
    mu_run_test(test_expr_variable_eval_array_with_cond_unshared2);
    mu_run_test(test_expr_variable_tostring_non_array);
    mu_run_test(test_expr_variable_tostring_array);
    mu_run_test(test_expr_variable_tostring_non_array_by_name);
    mu_run_test(test_expr_variable_tostring_array_by_name);
    mu_run_test(test_expr_variable_equal1a);
    mu_run_test(test_expr_variable_equal1b);
    mu_run_test(test_expr_variable_equal2a);
    mu_run_test(test_expr_variable_equal2b);
    mu_run_test(test_expr_variable_equal3a);
    mu_run_test(test_expr_variable_equal3b);
    mu_run_test(test_expr_variable_equal4);
    mu_run_test(test_expr_variable_clone1a);
    mu_run_test(test_expr_variable_clone1b);
    mu_run_test(test_expr_variable_clone2a);
    mu_run_test(test_expr_variable_clone2b);
}
