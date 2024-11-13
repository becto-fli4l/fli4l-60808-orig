/*****************************************************************************
 *  @file test_expr_typechecker.c
 *  Functions for testing the type checker for expressions.
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

#define MU_TESTSUITE "expression.expr_typechecker"

/**
 * Tests the type checker on a literal expression.
 */
mu_test_begin(test_expr_typechecker_literal)
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Error"));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an unresolved variable expression which can be
 * resolved.
 */
mu_test_begin(test_expr_typechecker_variable_unresolved1)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an unresolved variable expression which cannot be
 * resolved.
 */
mu_test_begin(test_expr_typechecker_variable_unresolved2)
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a resolved variable expression.
 */
mu_test_begin(test_expr_typechecker_variable_resolved)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical NOT expression that encapsulates a
 * type-conformant subexpression.
 */
mu_test_begin(test_expr_typechecker_logical_not1)
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical NOT expression that encapsulates a
 * subexpression that is not type-conformant.
 */
mu_test_begin(test_expr_typechecker_logical_not2)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical NOT expression that encapsulates a
 * non-boolean subexpression.
 */
mu_test_begin(test_expr_typechecker_logical_not3)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical AND expression that encapsulates two
 * type-conformant subexpressions.
 */
mu_test_begin(test_expr_typechecker_logical_and1)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical AND expression that encapsulates a
 * subexpression that is not type-conformant (1).
 */
mu_test_begin(test_expr_typechecker_logical_and2)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr1);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical AND expression that encapsulates a
 * subexpression that is not type-conformant (2).
 */
mu_test_begin(test_expr_typechecker_logical_and3)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical AND expression that encapsulates a
 * non-boolean subexpression (1).
 */
mu_test_begin(test_expr_typechecker_logical_and4)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical AND expression that encapsulates a
 * non-boolean subexpression (2).
 */
mu_test_begin(test_expr_typechecker_logical_and5)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical OR expression that encapsulates two
 * type-conformant subexpressions.
 */
mu_test_begin(test_expr_typechecker_logical_or1)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical OR expression that encapsulates a
 * subexpression that is not type-conformant (1).
 */
mu_test_begin(test_expr_typechecker_logical_or2)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr1);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical OR expression that encapsulates a
 * subexpression that is not type-conformant (2).
 */
mu_test_begin(test_expr_typechecker_logical_or3)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical OR expression that encapsulates a
 * non-boolean subexpression (1).
 */
mu_test_begin(test_expr_typechecker_logical_or4)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a logical OR expression that encapsulates a
 * non-boolean subexpression (2).
 */
mu_test_begin(test_expr_typechecker_logical_or5)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression that encapsulates two
 * type-conformant and type-identical subexpressions.
 */
mu_test_begin(test_expr_typechecker_equal1)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression that encapsulates two
 * type-conformant subexpressions where the left one is assignable to the
 * right one.
 */
mu_test_begin(test_expr_typechecker_equal2)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression that encapsulates two
 * type-conformant subexpressions where the right one is assignable to the
 * left one.
 */
mu_test_begin(test_expr_typechecker_equal3)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression that encapsulates a
 * subexpression that is not type-conformant (1).
 */
mu_test_begin(test_expr_typechecker_equal4)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr1);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression that encapsulates a
 * subexpression that is not type-conformant (2).
 */
mu_test_begin(test_expr_typechecker_equal5)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression with subexpressions that
 * are not assignment-compatible (1).
 */
mu_test_begin(test_expr_typechecker_equal6)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an equality expression with subexpressions that
 * are not assignment-compatible (2).
 */
mu_test_begin(test_expr_typechecker_equal7)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression that encapsulates two
 * type-conformant and type-identical subexpressions.
 */
mu_test_begin(test_expr_typechecker_unequal1)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression that encapsulates two
 * type-conformant subexpressions where the left one is assignable to the
 * right one.
 */
mu_test_begin(test_expr_typechecker_unequal2)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression that encapsulates two
 * type-conformant subexpressions where the right one is assignable to the
 * left one.
 */
mu_test_begin(test_expr_typechecker_unequal3)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression that encapsulates a
 * subexpression that is not type-conformant (1).
 */
mu_test_begin(test_expr_typechecker_unequal4)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr1);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression that encapsulates a
 * subexpression that is not type-conformant (2).
 */
mu_test_begin(test_expr_typechecker_unequal5)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression with subexpressions that
 * are not assignment-compatible (1).
 */
mu_test_begin(test_expr_typechecker_unequal6)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    subexpr2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on an inequality expression with subexpressions that
 * are not assignment-compatible (2).
 */
mu_test_begin(test_expr_typechecker_unequal7)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    subexpr2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a match expression that encapsulates two
 * type-conformant subexpressions.
 */
mu_test_begin(test_expr_typechecker_match1)
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "H.*"));
    expr = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a match expression that encapsulates a
 * subexpression that is not type-conformant (1).
 */
mu_test_begin(test_expr_typechecker_match2)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    subexpr2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "H.*"));
    expr = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr1);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a match expression that encapsulates a
 * subexpression that is not type-conformant (2).
 */
mu_test_begin(test_expr_typechecker_match3)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr1;
    struct expr_t *subexpr2;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo"));
    subexpr2 = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        subexpr1,
        subexpr2
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a TOBOOL expression that encapsulates a
 * type-conformant subexpression.
 */
mu_test_begin(test_expr_typechecker_tobool1)
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();

    subexpr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_tobool_to_expr(expr_tobool_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(expr_typecheck_expression(expr, &error_expr));
    mu_assert(!error_expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a TOBOOL expression that encapsulates a
 * subexpression that is not type-conformant.
 */
mu_test_begin(test_expr_typechecker_tobool2)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V"));
    expr = expr_tobool_to_expr(expr_tobool_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == subexpr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the type checker on a TOBOOL expression that encapsulates a
 * non-boolean subexpression.
 */
mu_test_begin(test_expr_typechecker_tobool3)
    struct vartype_t *type;
    struct var_t *var;
    struct expr_t *subexpr;
    struct expr_t *expr;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    subexpr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    expr = expr_tobool_to_expr(expr_tobool_create(
        var_get_global_scope(),
        subexpr
    ));
    mu_assert(!expr_typecheck_expression(expr, &error_expr));
    mu_assert(error_expr == expr);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_typechecker(void)
{
    mu_run_test(test_expr_typechecker_literal);
    mu_run_test(test_expr_typechecker_variable_unresolved1);
    mu_run_test(test_expr_typechecker_variable_unresolved2);
    mu_run_test(test_expr_typechecker_variable_resolved);
    mu_run_test(test_expr_typechecker_logical_not1);
    mu_run_test(test_expr_typechecker_logical_not2);
    mu_run_test(test_expr_typechecker_logical_not3);
    mu_run_test(test_expr_typechecker_logical_and1);
    mu_run_test(test_expr_typechecker_logical_and2);
    mu_run_test(test_expr_typechecker_logical_and3);
    mu_run_test(test_expr_typechecker_logical_and4);
    mu_run_test(test_expr_typechecker_logical_and5);
    mu_run_test(test_expr_typechecker_logical_or1);
    mu_run_test(test_expr_typechecker_logical_or2);
    mu_run_test(test_expr_typechecker_logical_or3);
    mu_run_test(test_expr_typechecker_logical_or4);
    mu_run_test(test_expr_typechecker_logical_or5);
    mu_run_test(test_expr_typechecker_equal1);
    mu_run_test(test_expr_typechecker_equal2);
    mu_run_test(test_expr_typechecker_equal3);
    mu_run_test(test_expr_typechecker_equal4);
    mu_run_test(test_expr_typechecker_equal5);
    mu_run_test(test_expr_typechecker_equal6);
    mu_run_test(test_expr_typechecker_equal7);
    mu_run_test(test_expr_typechecker_unequal1);
    mu_run_test(test_expr_typechecker_unequal2);
    mu_run_test(test_expr_typechecker_unequal3);
    mu_run_test(test_expr_typechecker_unequal4);
    mu_run_test(test_expr_typechecker_unequal5);
    mu_run_test(test_expr_typechecker_unequal6);
    mu_run_test(test_expr_typechecker_unequal7);
    mu_run_test(test_expr_typechecker_match1);
    mu_run_test(test_expr_typechecker_match2);
    mu_run_test(test_expr_typechecker_match3);
    mu_run_test(test_expr_typechecker_tobool1);
    mu_run_test(test_expr_typechecker_tobool2);
    mu_run_test(test_expr_typechecker_tobool3);
}
