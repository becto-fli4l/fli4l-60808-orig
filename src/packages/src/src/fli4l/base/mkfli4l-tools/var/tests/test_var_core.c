/*****************************************************************************
 *  @file test_var_core.c
 *  Functions for testing variables.
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
#include <var.h>
#include <expression.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.core"

/**
 * Tests adding a simple non-array variable.
 */
mu_test_begin(test_var_add_simple1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct location_t *location;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    mu_assert_eq_str("V", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var));
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(0, var_get_level(var));
    mu_assert(location == var_get_location(var));

    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);
    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a simple non-array variable with a default value.
 */
mu_test_begin(test_var_add_simple2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct location_t *location;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    mu_assert_eq_str("V", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(0, var_get_level(var));
    mu_assert(location == var_get_location(var));

    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding an optional simple non-array variable with a default value.
 */
mu_test_begin(test_var_add_simple3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->optional = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    mu_assert(var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array.
 */
mu_test_begin(test_var_add_array1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, location, props);
    mu_assert(var);
    mu_assert_eq_str("V_%", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert_eq_int(VAR_PRIORITY_SCRIPT_TRANSIENT, var_get_priority(var));
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(1, var_get_level(var));
    mu_assert(location == var_get_location(var));

    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);
    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert_eq_int(VAR_PRIORITY_SCRIPT_TRANSIENT, var_get_priority(var_n));
    mu_assert(var_get_scope(var_n) == var_get_global_scope());
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);
    mu_assert(propsNew->weak);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with a default value.
 */
mu_test_begin(test_var_add_array1_def)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3)));
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    mu_assert_eq_str("V_%", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(1, var_get_level(var));
    mu_assert(location == var_get_location(var));

    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 3);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert(var_get_scope(var_n) == var_get_global_scope());
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);
    mu_assert(propsNew->weak);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with a wrong name.
 */
mu_test_begin(test_var_add_array1_wrong)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V(%", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a three-dimensional array.
 */
mu_test_begin(test_var_add_array2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    mu_assert_eq_str("V_%_A_%_B_%_C", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(3, var_get_level(var));
    mu_assert(location == var_get_location(var));

    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);
    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert(var_get_scope(var_n) == var_get_global_scope());
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);
    mu_assert(propsNew->weak);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    var_n = var_get("V_%_A_N");
    mu_assert(var_n);
    mu_assert(var_get_scope(var_n) == var_get_global_scope());
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);
    mu_assert(propsNew->weak);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    var_n = var_get("V_%_A_%_B_N");
    mu_assert(var_n);
    mu_assert(var_get_scope(var_n) == var_get_global_scope());
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);
    mu_assert(propsNew->weak);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound.
 */
mu_test_begin(test_var_add_array3)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *cond;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    expr_destroy(expr_true);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different enabling conditions.
 */
mu_test_begin(test_var_add_array4)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *expr_cond1;
    struct expr_t *expr_cond2;
    struct expr_t *expr_cond_n;
    struct expr_t *cond;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    expr_cond1 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))));
    var_properties_set_condition(props1, expr_cond1);
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    expr_cond2 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))));
    var_properties_set_condition(props2, expr_cond2);
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    expr_cond_n = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
                    expr_tobool_to_expr(expr_tobool_create(var_get_global_scope(),
                        expr_clone(expr_cond1))),
                    expr_clone(expr_cond2)));
    mu_assert(expr_typecheck_expression(expr_cond_n, &error_expr));
    mu_assert(expr_equal(expr_cond_n, cond));
    expr_destroy(expr_cond_n);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different enabling conditions where the array bound is explicitly
 * defined and has an own condition.
 */
mu_test_begin(test_var_add_array5)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_n;
    struct expr_t *expr_cond1;
    struct expr_t *expr_cond2;
    struct expr_t *expr_cond_and;
    struct expr_t *expr_cond_n;
    struct expr_t *cond;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    expr_cond_and = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
    ));
    var_properties_set_condition(props, expr_cond_and);
    var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props);
    props = var_properties_create();
    expr_cond1 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))));
    var_properties_set_condition(props, expr_cond1);
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    props = var_properties_create();
    expr_cond2 = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))));
    var_properties_set_condition(props, expr_cond2);
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    expr_cond_n = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        expr_clone(expr_cond_and),
        expr_logical_or_to_expr(expr_logical_or_create(
            var_get_global_scope(),
            expr_tobool_to_expr(expr_tobool_create(var_get_global_scope(), expr_clone(expr_cond1))),
            expr_clone(expr_cond2)
        ))
    ));
    mu_assert(expr_typecheck_expression(expr_cond_n, &error_expr));
    mu_assert(expr_equal(expr_cond_n, cond));
    expr_destroy(expr_cond_n);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with an incompatible array bound.
 */
mu_test_begin(test_var_add_array_incompatible)
    struct vartype_t *type_none;
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type_none = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_N", type_none, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props1);
    props2 = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, loc, props2);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with a compatible explicit array bound.
 */
mu_test_begin(test_var_add_array_explicit_bound1)
    struct vartype_t *type_numeric;
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type_numeric = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_N", type_numeric, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props1);
    props2 = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add_with_array_bound(var_get_global_scope(), "V_%", "V_N", type, VAR_PRIORITY_CONFIGURATION, loc, props2);
    mu_assert(var);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with an incompatible explicit array
 * bound (1).
 */
mu_test_begin(test_var_add_array_explicit_bound2)
    struct vartype_t *type_numeric;
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type_numeric = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_N", type_numeric, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props1);
    props2 = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add_with_array_bound(var_get_global_scope(), "V_%_%", "V_N", type, VAR_PRIORITY_CONFIGURATION, loc, props2);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with an incompatible explicit array
 * bound (2).
 */
mu_test_begin(test_var_add_array_explicit_bound3)
    struct vartype_t *type_numeric;
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type_numeric = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_%_%_N", type_numeric, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props1);
    props2 = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add_with_array_bound(var_get_global_scope(), "V_%_%", "V_%_%_N", type, VAR_PRIORITY_CONFIGURATION, loc, props2);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with an incompatible explicit array
 * bound (3).
 */
mu_test_begin(test_var_add_array_explicit_bound4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct location_t *loc;
    struct var_t *var;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add_with_array_bound(var_get_global_scope(), "V_%_%", "V_%_%_N", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a non-array variable with an explicit array bound.
 */
mu_test_begin(test_var_add_nonarray_explicit_bound)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct location_t *loc;
    struct var_t *var;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add_with_array_bound(var_get_global_scope(), "V", "V_%_N", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests var_add(var_get_global_scope(), ) using wrong names.
 */
mu_test_begin(test_var_add_wrong)
    struct vartype_t *type;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    props = var_properties_create();
    mu_assert(!var_add(var_get_global_scope(), "V_._%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_()_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_[A]_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_A{4}_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_?_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_x*_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_y+_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_^^_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_$$_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "%%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "%V", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V%", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_%X", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V%_X", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_%%_X", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    mu_assert(!var_add(var_get_global_scope(), "V_%X%_X", type, VAR_PRIORITY_CONFIGURATION, loc, props));
    var_properties_destroy(props);
    location_destroy(loc);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests removing a variable.
 */
mu_test_begin(test_var_remove)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *location;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == var_get_global_scope());

    var2 = var_get("V");
    mu_assert(var2 == var1);

    var_remove(var1);
    var2 = var_get("V");
    mu_assert(var2 == NULL);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests retrieving a non-existing variable.
 */
mu_test_begin(test_var_get_non_existing)
    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();
    mu_assert(!var_get("V"));
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting identical variable properties.
 */
mu_test_begin(test_var_change_props1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);

    props = var_properties_clone(var_get_properties(var));
    mu_assert(var_set_properties(var, props));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting the "optional" property of an array with non-optional array
 * bound.
 */
mu_test_begin(test_var_change_props2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->optional);

    props = var_properties_clone(propsNew);
    props->optional = TRUE;
    mu_assert(var_set_properties(var, props));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests changing a default value.
 */
mu_test_begin(test_var_change_props3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct location_t *location;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);
    props = var_properties_clone(propsNew);
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2)));
    mu_assert(var_set_properties(var, props));
    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 2);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    props = var_properties_clone(propsNew);
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3)));
    mu_assert(var_set_properties(var_n, props));
    propsNew = var_get_properties(var_n);
    def_value = expr_literal_create_integer(var_get_global_scope(), 3);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests changing a condition.
 */
mu_test_begin(test_var_change_props4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *expr_false
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    struct expr_t *cond;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);

    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_true, cond));

    props = var_properties_clone(var_get_properties(var));
    var_properties_set_condition(props, expr_clone(expr_false));
    mu_assert(var_set_properties(var, props));

    mu_assert(var_has_valid_condition(var_n));
    cond = var_get_condition(var_n);
    mu_assert(expr_equal(expr_false, cond));

    expr_destroy(expr_true);
    expr_destroy(expr_false);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting both "optional" and a default value.
 */
mu_test_begin(test_var_change_props5)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);

    props = var_properties_clone(var_get_properties(var));
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    mu_assert(var_set_properties(var, props));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (1).
 */
mu_test_begin(test_var_invalid_cond1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *expr_false
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    struct expr_t *cond;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);

    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    props = var_properties_clone(var_get_properties(var));
    var_properties_set_condition(
        props,
        expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "XYZ"))
    );
    mu_assert(var_set_properties(var, props));

    mu_assert(!var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_false, cond));

    expr_destroy(expr_true);
    expr_destroy(expr_false);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (2).
 */
mu_test_begin(test_var_invalid_cond2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *expr_false
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    struct expr_t *cond;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);

    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    props = var_properties_clone(var_get_properties(var));
    var_properties_set_condition(
        props,
        expr_equal_to_expr(expr_equal_create(
            var_get_global_scope(),
            expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123)),
            expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
        ))
    );
    mu_assert(var_set_properties(var, props));

    mu_assert(!var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_false, cond));

    expr_destroy(expr_true);
    expr_destroy(expr_false);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (3).
 */
mu_test_begin(test_var_invalid_cond3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *expr_false
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    struct expr_t *cond;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);

    mu_assert(var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_true, cond));

    props = var_properties_clone(var_get_properties(var));
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123))
    );
    mu_assert(var_set_properties(var, props));

    mu_assert(!var_has_valid_condition(var));
    cond = var_get_condition(var);
    mu_assert(expr_equal(expr_false, cond));

    expr_destroy(expr_true);
    expr_destroy(expr_false);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_core(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_add_simple1);
    mu_run_test(test_var_add_simple2);
    mu_run_test(test_var_add_simple3);
    mu_run_test(test_var_add_array1);
    mu_run_test(test_var_add_array1_def);
    mu_run_test(test_var_add_array1_wrong);
    mu_run_test(test_var_add_array2);
    mu_run_test(test_var_add_array3);
    mu_run_test(test_var_add_array4);
    mu_run_test(test_var_add_array5);
    mu_run_test(test_var_add_array_incompatible);
    mu_run_test(test_var_add_array_explicit_bound1);
    mu_run_test(test_var_add_array_explicit_bound2);
    mu_run_test(test_var_add_array_explicit_bound3);
    mu_run_test(test_var_add_array_explicit_bound4);
    mu_run_test(test_var_add_nonarray_explicit_bound);
    mu_run_test(test_var_add_wrong);
    mu_run_test(test_var_remove);
    mu_run_test(test_var_get_non_existing);
    mu_run_test(test_var_change_props1);
    mu_run_test(test_var_change_props2);
    mu_run_test(test_var_change_props3);
    mu_run_test(test_var_change_props4);
    mu_run_test(test_var_change_props5);
    mu_run_test(test_var_invalid_cond1);
    mu_run_test(test_var_invalid_cond2);
    mu_run_test(test_var_invalid_cond3);
}
