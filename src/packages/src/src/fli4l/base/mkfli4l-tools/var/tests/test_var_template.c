/*****************************************************************************
 *  @file test_var_template.c
 *  Functions for testing variable templates.
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
#include <vartype.h>
#include <var.h>
#include <expression.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.template"

/**
 * Tests adding a variable template to and removing it from the global scope.
 */
mu_test_begin(test_var_template_add_global)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_template_t *tmpl2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    tmpl = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl);
    tmpl = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl);
    tmpl = var_scope_try_get_template(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!tmpl);
    tmpl = var_scope_get_template(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!tmpl);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    mu_assert_eq_str(".*_DO_DEBUG", var_template_get_name(tmpl));
    mu_assert(var_template_get_type(tmpl) == type);
    mu_assert(var_template_get_scope(tmpl) == scope);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_template_get_priority(tmpl));
    mu_assert(location_equals(location, var_template_get_location(tmpl)));

    propsNew = var_template_get_properties(tmpl);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    tmpl2 = var_template_try_get(".*_DO_DEBUG");
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_template_get(".*_DO_DEBUG");
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);

    var_template_remove(tmpl);

    tmpl2 = var_template_try_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(!tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(!tmpl2);
    tmpl2 = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a variable template to and removing it from a nested scope.
 */
mu_test_begin(test_var_template_add_nested)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_template_t *tmpl2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    tmpl = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl);
    tmpl = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl);
    tmpl = var_scope_try_get_template(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!tmpl);
    tmpl = var_scope_get_template(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!tmpl);
    tmpl = var_scope_try_get_template(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!tmpl);
    tmpl = var_scope_get_template(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!tmpl);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    mu_assert_eq_str(".*_DO_DEBUG", var_template_get_name(tmpl));
    mu_assert(var_template_get_type(tmpl) == type);
    mu_assert(var_template_get_scope(tmpl) == scope);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_template_get_priority(tmpl));
    mu_assert(location_equals(location, var_template_get_location(tmpl)));

    propsNew = var_template_get_properties(tmpl);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    tmpl2 = var_template_try_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", TRUE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", TRUE);
    mu_assert(tmpl == tmpl2);
    tmpl2 = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);

    var_template_remove(tmpl);

    tmpl2 = var_template_try_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get(".*_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(!tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", FALSE);
    mu_assert(!tmpl2);
    tmpl2 = var_scope_try_get_template(scope, ".*_DO_DEBUG", TRUE);
    mu_assert(!tmpl2);
    tmpl2 = var_scope_get_template(scope, ".*_DO_DEBUG", TRUE);
    mu_assert(!tmpl2);
    tmpl2 = var_template_try_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);
    tmpl2 = var_template_get("ABC_DO_DEBUG");
    mu_assert(!tmpl2);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding an optional variable template with a default value.
 */
mu_test_begin(test_var_template_add_optional_with_default)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();

    props = var_properties_create();
    props->optional = TRUE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining a variable template in the global scope.
 */
mu_test_begin(test_var_template_redefine_global)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t *props2;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;
    struct location_t *location2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    props2 = var_properties_clone(props);
    location2 = location_clone(location);
    mu_assert(!var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location2, props2));
    location_destroy(location2);
    var_properties_destroy(props2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining a variable template in a nested scope.
 */
mu_test_begin(test_var_template_redefine_nested)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t *props2;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;
    struct location_t *location2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    props2 = var_properties_clone(props);
    location2 = location_clone(location);
    mu_assert(!var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location2, props2));
    location_destroy(location2);
    var_properties_destroy(props2);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template in the global scope.
 */
mu_test_begin(test_var_template_inst_global)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct var_t *var2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(var);
    mu_assert_eq_str("ABC_DO_DEBUG", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_template(var) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var));
    mu_assert(var_get_scope(var) == scope);
    mu_assert_eq_int(0, var_get_level(var));
    mu_assert(location_equals(location, var_get_location(var)));

    propsNew = var_get_properties(var);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var2 = var_try_get("ABC_DO_DEBUG");
    mu_assert(var == var2);
    var2 = var_get("ABC_DO_DEBUG");
    mu_assert(var == var2);
    var2 = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var == var2);
    var2 = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var == var2);

    var_template_remove(tmpl);
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template in a nested scope.
 */
mu_test_begin(test_var_template_inst_nested)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct var_t *var2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var);
    mu_assert_eq_str("ABC_DO_DEBUG", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_template(var) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var));
    mu_assert(var_get_scope(var) == scope);
    mu_assert_eq_int(0, var_get_level(var));
    mu_assert(location_equals(location, var_get_location(var)));

    propsNew = var_get_properties(var);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var2 = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var2);
    var2 = var_get("ABC_DO_DEBUG");
    mu_assert(!var2);
    var2 = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var == var2);
    var2 = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var == var2);
    var2 = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(var == var2);
    var2 = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(var == var2);

    var_template_remove(tmpl);
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template in a parent scope.
 */
mu_test_begin(test_var_template_inst_parent)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct var_t *var2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(var_get_global_scope(), ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(var);
    mu_assert_eq_str("ABC_DO_DEBUG", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert(var_get_template(var) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var));
    mu_assert(var_get_scope(var) == var_get_global_scope());
    mu_assert_eq_int(0, var_get_level(var));
    mu_assert(location_equals(location, var_get_location(var)));

    propsNew = var_get_properties(var);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var2 = var_try_get("ABC_DO_DEBUG");
    mu_assert(var == var2);
    var2 = var_get("ABC_DO_DEBUG");
    mu_assert(var == var2);
    var2 = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var2);
    var2 = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var2);
    var2 = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(var == var2);
    var2 = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(var == var2);

    var_template_remove(tmpl);
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", TRUE);
    mu_assert(!var);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template without wildcards.
 */
mu_test_begin(test_var_template_inst_no_wildcards)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(!var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(!var);

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, "ABC_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(!var);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template that cannot be instantiated.
 */
mu_test_begin(test_var_template_inst_failure)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct location_t *location;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    props = var_properties_create();
    mu_assert(var_add(scope, "V_N", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props));

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, "[^%]+_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var = var_try_get("X_%");
    mu_assert(var);
    var = var_try_get("V_%");
    mu_assert(!var);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template twice in the global scope.
 */
mu_test_begin(test_var_template_inst_twice1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    mu_assert(!var_try_get("ABC_DO_DEBUG"));
    mu_assert(!var_get("ABC_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_try_get("DEF_DO_DEBUG"));
    mu_assert(!var_get("DEF_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "DEF_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "DEF_DO_DEBUG", FALSE));

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var1 = var_try_get("ABC_DO_DEBUG");
    mu_assert(var1);
    mu_assert_eq_str("ABC_DO_DEBUG", var_get_name(var1));
    mu_assert(var_get_type(var1) == type);
    mu_assert(var_get_template(var1) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var1));
    mu_assert(var_get_scope(var1) == scope);
    mu_assert_eq_int(0, var_get_level(var1));
    mu_assert(location_equals(location, var_get_location(var1)));

    propsNew = var_get_properties(var1);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var2 = var_try_get("DEF_DO_DEBUG");
    mu_assert(var2);
    mu_assert_eq_str("DEF_DO_DEBUG", var_get_name(var2));
    mu_assert(var_get_type(var2) == type);
    mu_assert(var_get_template(var2) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var2));
    mu_assert(var_get_scope(var2) == scope);
    mu_assert_eq_int(0, var_get_level(var2));
    mu_assert(location_equals(location, var_get_location(var2)));

    propsNew = var_get_properties(var2);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(var1 == var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(var1 == var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var1 == var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var1 == var);

    var = var_try_get("DEF_DO_DEBUG");
    mu_assert(var2 == var);
    var = var_get("DEF_DO_DEBUG");
    mu_assert(var2 == var);
    var = var_scope_try_get(scope, "DEF_DO_DEBUG", FALSE);
    mu_assert(var2 == var);
    var = var_scope_get(scope, "DEF_DO_DEBUG", FALSE);
    mu_assert(var2 == var);

    var_template_remove(tmpl);

    mu_assert(!var_try_get("ABC_DO_DEBUG"));
    mu_assert(!var_get("ABC_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_try_get("DEF_DO_DEBUG"));
    mu_assert(!var_get("DEF_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "DEF_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "DEF_DO_DEBUG", FALSE));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a variable template twice in the global scope and
 * removing the instances before the template.
 */
mu_test_begin(test_var_template_inst_twice2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct var_t *var;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    mu_assert(!var_try_get("ABC_DO_DEBUG"));
    mu_assert(!var_get("ABC_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "ABC_DO_DEBUG", FALSE));
    mu_assert(!var_try_get("DEF_DO_DEBUG"));
    mu_assert(!var_get("DEF_DO_DEBUG"));
    mu_assert(!var_scope_try_get(scope, "DEF_DO_DEBUG", FALSE));
    mu_assert(!var_scope_get(scope, "DEF_DO_DEBUG", FALSE));

    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    var1 = var_try_get("ABC_DO_DEBUG");
    mu_assert(var1);
    mu_assert_eq_str("ABC_DO_DEBUG", var_get_name(var1));
    mu_assert(var_get_type(var1) == type);
    mu_assert(var_get_template(var1) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var1));
    mu_assert(var_get_scope(var1) == scope);
    mu_assert_eq_int(0, var_get_level(var1));
    mu_assert(location_equals(location, var_get_location(var1)));

    propsNew = var_get_properties(var1);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var2 = var_try_get("DEF_DO_DEBUG");
    mu_assert(var2);
    mu_assert_eq_str("DEF_DO_DEBUG", var_get_name(var2));
    mu_assert(var_get_type(var2) == type);
    mu_assert(var_get_template(var2) == tmpl);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var2));
    mu_assert(var_get_scope(var2) == scope);
    mu_assert_eq_int(0, var_get_level(var2));
    mu_assert(location_equals(location, var_get_location(var2)));

    propsNew = var_get_properties(var2);
    mu_assert_eq_int(props->optional, propsNew->optional);
    mu_assert_eq_int(props->weak, propsNew->weak);
    mu_assert(expr_equal(props->def_value, propsNew->def_value));
    mu_assert_eq_str(props->dis_value, propsNew->dis_value);
    mu_assert(expr_equal(props->condition, propsNew->condition));

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var = var_try_get("ABC_DO_DEBUG");
    mu_assert(var1 == var);
    var = var_get("ABC_DO_DEBUG");
    mu_assert(var1 == var);
    var = var_scope_try_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var1 == var);
    var = var_scope_get(scope, "ABC_DO_DEBUG", FALSE);
    mu_assert(var1 == var);

    var = var_try_get("DEF_DO_DEBUG");
    mu_assert(var2 == var);
    var = var_get("DEF_DO_DEBUG");
    mu_assert(var2 == var);
    var = var_scope_try_get(scope, "DEF_DO_DEBUG", FALSE);
    mu_assert(var2 == var);
    var = var_scope_get(scope, "DEF_DO_DEBUG", FALSE);
    mu_assert(var2 == var);

    var_remove(var2);
    var_remove(var1);
    var_template_remove(tmpl);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (1).
 */
mu_test_begin(test_var_template_invalid_cond1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "XYZ"))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(!var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (2).
 */
mu_test_begin(test_var_template_invalid_cond2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_equal_to_expr(expr_equal_create(
            scope,
            expr_literal_to_expr(expr_literal_create_integer(scope, 123)),
            expr_literal_to_expr(expr_literal_create_boolean(scope, TRUE))
        ))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(!var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests setting an invalid condition (3).
 */
mu_test_begin(test_var_template_invalid_cond3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_template_t *tmpl;
    struct location_t *location;
    struct expr_t *cond;
    struct expr_t *expected;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    scope = var_get_global_scope();
    props = var_properties_create();
    props->optional = FALSE;
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var_properties_set_disabled_value(props, "23");
    var_properties_set_comment(props, "blabla");
    var_properties_set_condition(
        props,
        expr_literal_to_expr(expr_literal_create_integer(scope, 123))
    );
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    tmpl = var_template_add(scope, ".*_DO_DEBUG", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(tmpl);

    expected = expr_literal_to_expr(expr_literal_create_boolean(scope, FALSE));
    mu_assert(!var_template_has_valid_condition(tmpl));
    cond = var_template_get_condition(tmpl);
    mu_assert(expr_equal(expected, cond));
    expr_destroy(expected);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

static BOOL
test_var_template_scope_foreach_visitor1(void *entry, void *data)
{
    ++*(int *) data;
    return TRUE;
    UNUSED(entry);
}

static BOOL
test_var_template_scope_foreach_visitor2(void *entry, void *data)
{
    return FALSE;
    UNUSED(data);
    UNUSED(entry);
}

/**
 * Tests var_scope_foreach_template().
 */
mu_test_begin(test_var_template_foreach)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_template_t *tmpl_global;
    struct var_template_t *tmpl_local;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    tmpl_global = var_template_add(var_get_global_scope(), "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(tmpl_global);
    tmpl_local = var_template_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(tmpl_local);

    i = 0;
    var_scope_foreach_template(scope, TRUE, test_var_template_scope_foreach_visitor1, &i);
    mu_assert_eq_int(2, i);
    i = 0;
    var_scope_foreach_template(scope, FALSE, test_var_template_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach_template(scope, TRUE, test_var_template_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);
    var_scope_foreach_template(scope, FALSE, test_var_template_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);

    i = 0;
    var_scope_foreach_template(var_get_global_scope(), TRUE, test_var_template_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach_template(var_get_global_scope(), FALSE, test_var_template_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach_template(var_get_global_scope(), TRUE, test_var_template_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);
    var_scope_foreach_template(var_get_global_scope(), FALSE, test_var_template_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_template(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_template_add_global);
    mu_run_test(test_var_template_add_nested);
    mu_run_test(test_var_template_add_optional_with_default);
    mu_run_test(test_var_template_redefine_global);
    mu_run_test(test_var_template_redefine_nested);
    mu_run_test(test_var_template_inst_global);
    mu_run_test(test_var_template_inst_nested);
    mu_run_test(test_var_template_inst_parent);
    mu_run_test(test_var_template_inst_no_wildcards);
    mu_run_test(test_var_template_inst_failure);
    mu_run_test(test_var_template_inst_twice1);
    mu_run_test(test_var_template_inst_twice2);
    mu_run_test(test_var_template_invalid_cond1);
    mu_run_test(test_var_template_invalid_cond2);
    mu_run_test(test_var_template_invalid_cond3);
    mu_run_test(test_var_template_foreach);
}
