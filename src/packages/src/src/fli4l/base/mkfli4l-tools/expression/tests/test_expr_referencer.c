/*****************************************************************************
 *  @file test_expr_referencer.c
 *  Functions for testing the expression referencer.
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

#include <testing/tests.h>
#include <expression.h>
#include <package.h>
#include <vartype.h>
#include <var.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr_referencer"

/**
 * Tests the referencer on a literal expression.
 */
mu_test_begin(test_expr_referencer_literal)
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Error"));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on a resolved variable expression.
 */
mu_test_begin(test_expr_referencer_variable_resolved)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved.
 */
mu_test_begin(test_expr_referencer_variable_unresolved1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_variable_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_create_by_name(var_get_global_scope(), "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(expr));
    mu_assert(expr_resolve_variable_expressions(expr_variable_to_expr(expr), &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(expr));
    mu_assert(expr_variable_get_resolved_instance(expr)->var == var);
    expr_destroy(expr_variable_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved.
 */
mu_test_begin(test_expr_referencer_variable_unresolved2)
    struct vartype_t *type;
    struct expr_variable_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr = expr_variable_create_by_name(var_get_global_scope(), "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(expr));
    mu_assert(!expr_resolve_variable_expressions(expr_variable_to_expr(expr), &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(expr));
    expr_destroy(expr_variable_to_expr(expr));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a logical NOT expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_not1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_not_to_expr(expr_logical_not_create(
        scope,
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a logical NOT expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_not2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_not_to_expr(expr_logical_not_create(
        scope,
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a logical AND expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_and1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a logical AND expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_and2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        scope,
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a logical AND expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_and3)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a logical AND expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_and4)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        scope,
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a logical OR expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_or1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a logical OR expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_or2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        scope,
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a logical OR expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_or3)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a logical OR expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_or4)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        scope,
        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within an equality comparison expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_equal1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_equal_to_expr(expr_equal_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_integer(scope, 42))
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within an equality comparison expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_equal2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_equal_to_expr(expr_equal_create(
        scope,
        expr_literal_to_expr(expr_literal_create_integer(scope, 42)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within an equality comparison expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_equal3)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_equal_to_expr(expr_equal_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_integer(scope, 42))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within an equality comparison expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_equal4)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_equal_to_expr(expr_equal_create(
        scope,
        expr_literal_to_expr(expr_literal_create_integer(scope, 42)),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a match expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_match1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_match_to_expr(expr_match_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_string(scope, ".*"))
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a match expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_match2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_match_to_expr(expr_match_create(
        scope,
        expr_literal_to_expr(expr_literal_create_string(scope, "Hallo")),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a match expression (1).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_match3)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_match_to_expr(expr_match_create(
        scope,
        expr_variable_to_expr(varexpr),
        expr_literal_to_expr(expr_literal_create_string(scope, ".*"))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a match expression (2).
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_match4)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_match_to_expr(expr_match_create(
        scope,
        expr_literal_to_expr(expr_literal_create_string(scope, "Hallo")),
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which can be
 * resolved and which is buried within a TOBOOL expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_tobool1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "V");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_tobool_to_expr(expr_tobool_create(
        scope,
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr));
    mu_assert(expr_variable_get_resolved_instance(varexpr)->var == var);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on an unresolved variable expression which cannot be
 * resolved and which is buried within a TOBOOL expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_deep_tobool2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr = expr_tobool_to_expr(expr_tobool_create(
        scope,
        expr_variable_to_expr(varexpr)
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on two unresolved variable expressions which can be
 * resolved and which are buried deeply within an expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_very_deep1)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct expr_variable_t *varexpr1;
    struct expr_variable_t *varexpr2;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var1 = var_add(scope, "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var2 = var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr1 = expr_variable_create_by_name(scope, "V1");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr1));
    varexpr2 = expr_variable_create_by_name(scope, "V2");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr2));

    expr = expr_logical_not_to_expr(expr_logical_not_create(
        scope,
        expr_tobool_to_expr(expr_tobool_create(
            scope,
            expr_logical_and_to_expr(expr_logical_and_create(
                scope,
                expr_logical_or_to_expr(expr_logical_or_create(
                    scope,
                    expr_equal_to_expr(expr_equal_create(
                        scope,
                        expr_unequal_to_expr(expr_unequal_create(
                            scope,
                            expr_match_to_expr(expr_match_create(
                                scope,
                                expr_variable_to_expr(varexpr1),
                                expr_literal_to_expr(expr_literal_create_string(scope, "Hallo.*"))
                            )),
                            expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                        )),
                        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                    )),
                    expr_variable_to_expr(varexpr2)
                )),
                expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
            ))
        ))
    ));
    mu_assert(expr_resolve_variable_expressions(expr, &error));
    mu_assert(!error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr1));
    mu_assert(expr_variable_get_resolved_instance(varexpr1)->var == var1);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr2));
    mu_assert(expr_variable_get_resolved_instance(varexpr2)->var == var2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on two unresolved variable expressions where the first
 * one cannot be resolved and which are buried deeply within an expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_very_deep2)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct expr_variable_t *varexpr1;
    struct expr_variable_t *varexpr2;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var_add(scope, "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr1 = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr1));
    varexpr2 = expr_variable_create_by_name(scope, "V2");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr2));

    expr = expr_logical_not_to_expr(expr_logical_not_create(
        scope,
        expr_tobool_to_expr(expr_tobool_create(
            scope,
            expr_logical_and_to_expr(expr_logical_and_create(
                scope,
                expr_logical_or_to_expr(expr_logical_or_create(
                    scope,
                    expr_equal_to_expr(expr_equal_create(
                        scope,
                        expr_unequal_to_expr(expr_unequal_create(
                            scope,
                            expr_match_to_expr(expr_match_create(
                                scope,
                                expr_variable_to_expr(varexpr1),
                                expr_literal_to_expr(expr_literal_create_string(scope, "Hallo.*"))
                            )),
                            expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                        )),
                        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                    )),
                    expr_variable_to_expr(varexpr2)
                )),
                expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
            ))
        ))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr1));
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr2));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the referencer on two unresolved variable expressions where the second
 * one cannot be resolved and which are buried deeply within an expression.
 */
mu_test_begin(test_expr_referencer_variable_unresolved_very_deep3)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct expr_variable_t *varexpr1;
    struct expr_variable_t *varexpr2;
    struct expr_t *expr;
    char const *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();
    var1 = var_add(scope, "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    varexpr1 = expr_variable_create_by_name(scope, "V1");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr1));
    varexpr2 = expr_variable_create_by_name(scope, "X");
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr2));

    expr = expr_logical_not_to_expr(expr_logical_not_create(
        scope,
        expr_tobool_to_expr(expr_tobool_create(
            scope,
            expr_logical_and_to_expr(expr_logical_and_create(
                scope,
                expr_logical_or_to_expr(expr_logical_or_create(
                    scope,
                    expr_equal_to_expr(expr_equal_create(
                        scope,
                        expr_unequal_to_expr(expr_unequal_create(
                            scope,
                            expr_match_to_expr(expr_match_create(
                                scope,
                                expr_variable_to_expr(varexpr1),
                                expr_literal_to_expr(expr_literal_create_string(scope, "Hallo.*"))
                            )),
                            expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                        )),
                        expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
                    )),
                    expr_variable_to_expr(varexpr2)
                )),
                expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
            ))
        ))
    ));
    mu_assert(!expr_resolve_variable_expressions(expr, &error));
    mu_assert_eq_str("X", error);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, expr_variable_get_state(varexpr1));
    mu_assert(expr_variable_get_resolved_instance(varexpr1)->var == var1);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, expr_variable_get_state(varexpr2));
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_referencer(void)
{
    mu_run_test(test_expr_referencer_literal);
    mu_run_test(test_expr_referencer_variable_resolved);
    mu_run_test(test_expr_referencer_variable_unresolved1);
    mu_run_test(test_expr_referencer_variable_unresolved2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_not1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_not2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_and1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_and2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_and3);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_and4);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_or1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_or2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_or3);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_or4);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_equal1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_equal2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_equal3);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_equal4);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_match1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_match2);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_match3);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_match4);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_tobool1);
    mu_run_test(test_expr_referencer_variable_unresolved_deep_tobool2);
    mu_run_test(test_expr_referencer_variable_unresolved_very_deep1);
    mu_run_test(test_expr_referencer_variable_unresolved_very_deep2);
    mu_run_test(test_expr_referencer_variable_unresolved_very_deep3);
}
