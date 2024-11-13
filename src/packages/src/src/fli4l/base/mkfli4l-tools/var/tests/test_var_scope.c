/*****************************************************************************
 *  @file test_var_scope.c
 *  Functions for testing variable scopes.
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

#define MU_TESTSUITE "var.scope"

/**
 * Tests the global scope.
 */
mu_test_begin(test_var_scope_global)
    struct vartype_t *type;
    struct var_t *var_global;
    struct var_t *var;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var_global = var_add(var_get_global_scope(), "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_global);
    mu_assert(var_get_scope(var_global) == var_get_global_scope());

    var = var_scope_get(var_get_global_scope(), "V1", FALSE);
    mu_assert(var == var_global);
    mu_assert(var_get_scope(var) == var_get_global_scope());
    var = var_scope_get(var_get_global_scope(), "V1", TRUE);
    mu_assert(var == var_global);
    mu_assert(var_get_scope(var) == var_get_global_scope());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a nested scope.
 */
mu_test_begin(test_var_scope_nested)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var_global;
    struct var_t *var_local;
    struct var_t *var;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    var_global = var_add(var_get_global_scope(), "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_global);
    mu_assert(var_get_scope(var_global) == var_get_global_scope());
    var_local = var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_local);
    mu_assert(var_get_scope(var_local) == scope);

    var = var_scope_get(scope, "V1", FALSE);
    mu_assert(!var);
    var = var_scope_get(scope, "V1", TRUE);
    mu_assert(var == var_global);
    var = var_scope_get(var_get_global_scope(), "V1", FALSE);
    mu_assert(var == var_global);
    var = var_scope_get(var_get_global_scope(), "V1", TRUE);
    mu_assert(var == var_global);

    var = var_scope_get(scope, "V2", FALSE);
    mu_assert(var == var_local);
    var = var_scope_get(scope, "V2", TRUE);
    mu_assert(var == var_local);
    var = var_scope_get(var_get_global_scope(), "V2", FALSE);
    mu_assert(!var);
    var = var_scope_get(var_get_global_scope(), "V2", TRUE);
    mu_assert(!var);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a non-weak non-array variable.
 */
mu_test_begin(test_var_add_redef_nonweak_simple)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var2);
    var_properties_destroy(props);
    location_destroy(loc);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a non-weak one-dimensional array variable.
 */
mu_test_begin(test_var_add_redef_nonweak_array)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V_%_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V_%_%_%", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var2);
    var_properties_destroy(props);
    location_destroy(loc);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak variable with another weak variable.
 */
mu_test_begin(test_var_add_redef_weak1)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    props->weak = TRUE;
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak variable with a non-weak variable.
 */
mu_test_begin(test_var_add_redef_weak2)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, optional variable with a non-weak, optional
 * variable.
 */
mu_test_begin(test_var_add_redef_weak_optional1)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    props->optional = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    props->optional = TRUE;
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, optional variable with a non-weak,
 * non-optional variable.
 */
mu_test_begin(test_var_add_redef_weak_optional2)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    props->optional = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * optional variable.
 */
mu_test_begin(test_var_add_redef_weak_optional3)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    props->optional = TRUE;
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable of another type.
 */
mu_test_begin(test_var_add_redef_weak_change_type)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(scope, "V", vartype_get(VARTYPE_PREDEFINED_ANYTHING), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable adding a default value.
 */
mu_test_begin(test_var_add_redef_weak_add_default)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), var_get_properties(var2)->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable with the same default value.
 */
mu_test_begin(test_var_add_redef_weak_unchanged_default)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), var_get_properties(var2)->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable changing a default value.
 */
mu_test_begin(test_var_add_redef_weak_change_default)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 1)));
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    def_value = expr_literal_create_integer(var_get_global_scope(), 1);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), var_get_properties(var2)->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable removing a default value.
 */
mu_test_begin(test_var_add_redef_weak_remove_default)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert(!var_get_properties(var2)->def_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable adding a disabled value.
 */
mu_test_begin(test_var_add_redef_weak_add_disabled)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_disabled_value(props, "0");
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert_eq_str("0", var_get_properties(var2)->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable with the same disabled value.
 */
mu_test_begin(test_var_add_redef_weak_unchanged_disabled)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_disabled_value(props, "0");
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_disabled_value(props, "0");
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert_eq_str("0", var_get_properties(var2)->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable changing a disabled value.
 */
mu_test_begin(test_var_add_redef_weak_change_disabled)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_disabled_value(props, "0");
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var_properties_set_disabled_value(props, "1");
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert_eq_str("1", var_get_properties(var2)->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of a weak, non-optional variable with a non-weak,
 * non-optional variable removing a disabled value.
 */
mu_test_begin(test_var_add_redef_weak_remove_disabled)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct location_t *loc;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    props->weak = TRUE;
    var_properties_set_disabled_value(props, "0");
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert(!var_get_properties(var2)->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of non-weak variables in the same scope and the same
 * priority.
 */
mu_test_begin(test_var_scope_redef_nonweak_sameprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of non-weak variables in the same scope and a lower
 * priority.
 */
mu_test_begin(test_var_scope_redef_nonweak_lowerprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of non-weak variables in the same scope and a higher
 * priority.
 */
mu_test_begin(test_var_scope_redef_nonweak_higherprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_SCRIPT_PERSISTENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of weak variables in the same scope and the same priority.
 */
mu_test_begin(test_var_scope_redef_weak_sameprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    props->weak = FALSE;
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var2);
    mu_assert(var_get_scope(var2) == scope);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(var2));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of weak variables in the same scope and a lower priority.
 */
mu_test_begin(test_var_scope_redef_weak_lowerprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    props->weak = FALSE;
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition of weak variables in the same scope and a higher priority.
 */
mu_test_begin(test_var_scope_redef_weak_higherprio)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_properties_t *props;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    scope = var_get_global_scope();

    props = var_properties_create();
    props->weak = TRUE;
    var1 = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    mu_assert(var_get_scope(var1) == scope);

    props = var_properties_create();
    props->weak = FALSE;
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(scope, "V", type, VAR_PRIORITY_SCRIPT_PERSISTENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining implicit array bound (1).
 */
mu_test_begin(test_var_scope_redef_imp_array_bound1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct var_t *var_n2;
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
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(propsNew->weak);
    mu_assert(!propsNew->optional);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n2 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n2);
    propsNew = var_get_properties(var_n2);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining implicit array bound (2).
 */
mu_test_begin(test_var_scope_redef_imp_array_bound2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct var_t *var_n2;
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
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(propsNew->weak);
    mu_assert(!propsNew->optional);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    props = var_properties_create();
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n2 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(!var_n2);
    location_destroy(location);
    var_properties_destroy(props);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining implicit array bound (3).
 */
mu_test_begin(test_var_scope_redef_imp_array_bound3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct var_t *var_n2;
    struct location_t *location;
    struct expr_literal_t *def_value;

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
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(propsNew->weak);
    mu_assert(!propsNew->optional);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n2 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n2);
    propsNew = var_get_properties(var_n2);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining implicit array bound (4).
 */
mu_test_begin(test_var_scope_redef_imp_array_bound4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct var_t *var_n2;
    struct location_t *location;
    struct expr_literal_t *def_value;

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
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(propsNew->weak);
    mu_assert(!propsNew->optional);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    props = var_properties_create();
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n2 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n2);
    propsNew = var_get_properties(var_n2);
    mu_assert(!propsNew->weak);
    mu_assert(propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining explicit array bound (1).
 */
mu_test_begin(test_var_scope_redef_exp_array_bound1)
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
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining explicit array bound (2).
 */
mu_test_begin(test_var_scope_redef_exp_array_bound2)
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
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    props = var_properties_create();
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(!propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining explicit array bound (3).
 */
mu_test_begin(test_var_scope_redef_exp_array_bound3)
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
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    props = var_properties_create();
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(!var);
    location_destroy(location);
    var_properties_destroy(props);
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefining explicit array bound (4).
 */
mu_test_begin(test_var_scope_redef_exp_array_bound4)
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
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    props = var_properties_create();
    props->optional = TRUE;
    location = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, location, props);
    mu_assert(var);
    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->weak);
    mu_assert(propsNew->optional);
    mu_assert(!propsNew->def_value);
    mu_assert(!propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition in a nested scope.
 */
mu_test_begin(test_var_scope_redef_nested)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var_global;
    struct var_t *var_local;
    struct var_t *var;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    var_global = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_global);
    mu_assert(var_get_scope(var_global) == var_get_global_scope());
    var_local = var_add(scope, "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_local);
    mu_assert(var_get_scope(var_local) == scope);

    var = var_scope_get(scope, "V", FALSE);
    mu_assert(var == var_local);
    var = var_scope_get(scope, "V", TRUE);
    mu_assert(var == var_local);
    var = var_scope_get(var_get_global_scope(), "V", FALSE);
    mu_assert(var == var_global);
    var = var_scope_get(var_get_global_scope(), "V", TRUE);
    mu_assert(var == var_global);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

static BOOL
test_var_scope_foreach_visitor1(void *entry, void *data)
{
    ++*(int *) data;
    return TRUE;
    (void) entry;
}

static BOOL
test_var_scope_foreach_visitor2(void *entry, void *data)
{
    return FALSE;
    (void) data;
    (void) entry;
}

/**
 * Tests var_scope_foreach().
 */
mu_test_begin(test_var_scope_foreach)
    struct vartype_t *type;
    struct var_scope_t *scope;
    struct var_t *var_global;
    struct var_t *var_local;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    scope = var_scope_create(location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), var_get_global_scope());
    var_global = var_add(var_get_global_scope(), "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_global);
    var_local = var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_local);

    i = 0;
    var_scope_foreach(scope, TRUE, test_var_scope_foreach_visitor1, &i);
    mu_assert_eq_int(2, i);
    i = 0;
    var_scope_foreach(scope, FALSE, test_var_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach(scope, TRUE, test_var_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);
    var_scope_foreach(scope, FALSE, test_var_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);

    i = 0;
    var_scope_foreach(var_get_global_scope(), TRUE, test_var_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach(var_get_global_scope(), FALSE, test_var_scope_foreach_visitor1, &i);
    mu_assert_eq_int(1, i);
    i = 0;
    var_scope_foreach(var_get_global_scope(), TRUE, test_var_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);
    var_scope_foreach(var_get_global_scope(), FALSE, test_var_scope_foreach_visitor2, &i);
    mu_assert_eq_int(0, i);

    var_scope_destroy(scope);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_scope(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_scope_global);
    mu_run_test(test_var_scope_nested);
    mu_run_test(test_var_add_redef_nonweak_simple);
    mu_run_test(test_var_add_redef_nonweak_array);
    mu_run_test(test_var_add_redef_weak1);
    mu_run_test(test_var_add_redef_weak2);
    mu_run_test(test_var_add_redef_weak_optional1);
    mu_run_test(test_var_add_redef_weak_optional2);
    mu_run_test(test_var_add_redef_weak_optional3);
    mu_run_test(test_var_add_redef_weak_change_type);
    mu_run_test(test_var_add_redef_weak_add_default);
    mu_run_test(test_var_add_redef_weak_unchanged_default);
    mu_run_test(test_var_add_redef_weak_change_default);
    mu_run_test(test_var_add_redef_weak_remove_default);
    mu_run_test(test_var_add_redef_weak_add_disabled);
    mu_run_test(test_var_add_redef_weak_unchanged_disabled);
    mu_run_test(test_var_add_redef_weak_change_disabled);
    mu_run_test(test_var_add_redef_weak_remove_disabled);
    mu_run_test(test_var_scope_redef_nonweak_sameprio);
    mu_run_test(test_var_scope_redef_nonweak_lowerprio);
    mu_run_test(test_var_scope_redef_nonweak_higherprio);
    mu_run_test(test_var_scope_redef_weak_sameprio);
    mu_run_test(test_var_scope_redef_weak_lowerprio);
    mu_run_test(test_var_scope_redef_weak_higherprio);
    mu_run_test(test_var_scope_redef_imp_array_bound1);
    mu_run_test(test_var_scope_redef_imp_array_bound2);
    mu_run_test(test_var_scope_redef_imp_array_bound3);
    mu_run_test(test_var_scope_redef_imp_array_bound4);
    mu_run_test(test_var_scope_redef_exp_array_bound1);
    mu_run_test(test_var_scope_redef_exp_array_bound2);
    mu_run_test(test_var_scope_redef_exp_array_bound3);
    mu_run_test(test_var_scope_redef_exp_array_bound4);
    mu_run_test(test_var_scope_redef_nested);
    mu_run_test(test_var_scope_foreach);
}
