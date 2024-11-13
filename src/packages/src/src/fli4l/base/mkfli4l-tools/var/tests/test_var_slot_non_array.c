/*****************************************************************************
 *  @file test_var_slot_non_array.c
 *  Functions for testing non-array variable slots.
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
#include <expression/expr_literal.h>
#include <expression/expr_variable.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.slot.non_array"

/**
 * Tests writing to and reading from a non-array variable.
 */
mu_test_begin(test_var_slot_rw_simple)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, NULL, NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    mu_assert(var_instance_set_value(inst, NULL, NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a non-array variable while changing only
 * the extra part.
 */
mu_test_begin(test_var_slot_rw_simple_change_extra)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);

    mu_assert(var_instance_set_and_check_value(inst, "1", value->extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);

    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);

    extra = var_slot_extra_create("def", NULL);
    mu_assert(var_instance_set_and_check_value(inst, value->value, extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);

    extra = var_slot_extra_create("ghi", NULL);
    mu_assert(var_instance_set_value(inst, NULL, extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a non-array variable while changing only
 * the value part.
 */
mu_test_begin(test_var_slot_rw_simple_change_value)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);

    mu_assert(var_instance_set_and_check_value(inst, "1", value->extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing NULL to a non-array variable.
 */
mu_test_begin(test_var_slot_rw_simple_null)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    mu_assert(!var_instance_set_and_check_value(inst, NULL, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));
    mu_assert(!var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests multiple writing to a non-array variable (tentative twice).
 */
mu_test_begin(test_var_slot_rw_simple_multi1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_SCRIPT_TRANSIENT, value->priority);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_int(VAR_PRIORITY_SCRIPT_TRANSIENT, value->priority);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests multiple writing to a non-array variable (final twice).
 */
mu_test_begin(test_var_slot_rw_simple_multi2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests multiple writing to a non-array variable (tentative, final).
 */
mu_test_begin(test_var_slot_rw_simple_multi3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_SCRIPT_TRANSIENT, value->priority);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests multiple writing to a non-array variable (final, tentative).
 */
mu_test_begin(test_var_slot_rw_simple_multi4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing a wrong value with checking.
 */
mu_test_begin(test_var_slot_rw_simple_wrong1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(!var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing a wrong value without checking.
 */
mu_test_begin(test_var_slot_rw_simple_wrong2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    mu_assert(var_instance_set_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
    var_value_destroy(value);

    mu_assert(!var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading an undefined variable.
 */
mu_test_begin(test_var_slot_r_simple_undef)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    value = var_instance_get_value(inst);
    mu_assert(!value);

    mu_assert(!var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading a disabled variable without a disabled value.
 */
mu_test_begin(test_var_slot_r_simple_disabled1)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_instance_t *inst_cond;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var_cond = var_add(var_get_global_scope(), "COND", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    inst_cond = var_instance_create(var_cond, 0, NULL);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    var_instance_set_value(inst_cond, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    value = var_instance_get_value(inst);
    mu_assert(!value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading a disabled variable with a disabled value that matches the
 * variable type.
 */
mu_test_begin(test_var_slot_r_simple_disabled2)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_instance_t *inst_cond;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var_cond = var_add(var_get_global_scope(), "COND", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    inst_cond = var_instance_create(var_cond, 0, NULL);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var_properties_set_disabled_value(props, VALUE_BOOLEAN_TRUE);
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    var_instance_set_value(inst_cond, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str(props->dis_value, value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading a disabled variable with a disabled value that does not match
 * the variable type.
 */
mu_test_begin(test_var_slot_r_simple_disabled3)
    struct vartype_t *type;
    struct var_t *var_cond;
    struct var_instance_t *inst_cond;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    var_cond = var_add(var_get_global_scope(), "COND", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    inst_cond = var_instance_create(var_cond, 0, NULL);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var_properties_set_disabled_value(props, "abc");
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);

    var_instance_set_value(inst_cond, VALUE_BOOLEAN_FALSE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str(props->dis_value, value->value);
    var_value_destroy(value);

    mu_assert(!var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading a variable with a condition depending on that variable.
 */
mu_test_begin(test_var_slot_r_simple_direct_cycle)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V")));
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_set_value(inst, VALUE_BOOLEAN_TRUE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(!var_instance_get_value(inst));
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading a variable with a condition depending on another variable
 * which itself depends on the first variable.
 */
mu_test_begin(test_var_slot_r_simple_indirect_cycle)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "W")));
    var1 = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);

    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V")));
    var2 = var_add(var_get_global_scope(), "W", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var2);

    inst = var_instance_create(var1, 0, NULL);
    mu_assert(var_instance_set_value(inst, VALUE_BOOLEAN_TRUE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);
    inst = var_instance_create(var2, 0, NULL);
    mu_assert(var_instance_set_value(inst, VALUE_BOOLEAN_TRUE, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var1, 0, NULL);
    mu_assert(!var_instance_get_value(inst));
    var_instance_destroy(inst);
    inst = var_instance_create(var2, 0, NULL);
    mu_assert(!var_instance_get_value(inst));
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading integer values from a variable.
 */
mu_test_begin(test_var_slot_rw_integer)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    int int_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_value(inst, "123X", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(!var_value_convert_to_integer(value, &int_value));
    var_value_destroy(value);

    mu_assert(var_instance_set_value(inst, "08", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(!var_value_convert_to_integer(value, &int_value));
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "123456789123456789123456789123456789", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(!var_value_convert_to_integer(value, &int_value));
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "123", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(var_value_convert_to_integer(value, &int_value));
    mu_assert_eq_int(123, int_value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading boolean values from a variable.
 */
mu_test_begin(test_var_slot_rw_boolean)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    BOOL bool_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(yes|no|maybe)", "'yes' or 'no' or 'maybe' expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(var_value_convert_to_boolean(value, &bool_value));
    mu_assert(bool_value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(var_value_convert_to_boolean(value, &bool_value));
    mu_assert(!bool_value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "maybe", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert(!var_value_convert_to_boolean(value, &bool_value));
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading from a non-array variable with default value.
 */
mu_test_begin(test_var_slot_r_simple_def)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_int(VAR_PRIORITY_UNDEFINED, value->priority);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests checking a missing value of an optional non-array variable.
 */
mu_test_begin(test_var_slot_r_simple_check_missing)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    props->optional = TRUE;
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));
    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_STRONG (1).
 */
mu_test_begin(test_var_slot_rw_strong1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_STRONG (2).
 */
mu_test_begin(test_var_slot_rw_strong2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(!value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_WEAK (1).
 */
mu_test_begin(test_var_slot_rw_weak1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_WEAK (2).
 */
mu_test_begin(test_var_slot_rw_weak2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(!var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(!value);

    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(!var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_TENTATIVE (1).
 */
mu_test_begin(test_var_slot_rw_tentative1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);

    extra = var_slot_extra_create("def", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);
    var_value_destroy(value);

    extra = var_slot_extra_create("ghi", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "4", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    extra = var_slot_extra_create("jkl", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "5", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests access priorities and VAR_ACCESS_TENTATIVE (2).
 */
mu_test_begin(test_var_slot_rw_tentative2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(!value);

    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(!value);

    extra = var_slot_extra_create("def", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);
    var_value_destroy(value);

    extra = var_slot_extra_create("ghi", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "4", extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SUPERSEDE));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    extra = var_slot_extra_create("jkl", NULL);
    mu_assert(var_instance_set_value(inst, NULL, extra, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    mu_assert_eq_str("ghi", (char const *) value->extra->data);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests checking the empty set of variables.
 */
mu_test_begin(test_var_slot_check_none)
    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests checking a disabled variable.
 */
mu_test_begin(test_var_slot_check_disabled)
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_condition(props, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var = var_add(var_get_global_scope(), "V", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 0, NULL);

    mu_assert(var_instance_set_value(inst, "abc", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_slot_non_array(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_slot_rw_simple);
    mu_run_test(test_var_slot_rw_simple_change_extra);
    mu_run_test(test_var_slot_rw_simple_change_value);
    mu_run_test(test_var_slot_rw_simple_null);
    mu_run_test(test_var_slot_rw_simple_multi1);
    mu_run_test(test_var_slot_rw_simple_multi2);
    mu_run_test(test_var_slot_rw_simple_multi3);
    mu_run_test(test_var_slot_rw_simple_multi4);
    mu_run_test(test_var_slot_rw_simple_wrong1);
    mu_run_test(test_var_slot_rw_simple_wrong2);
    mu_run_test(test_var_slot_r_simple_undef);
    mu_run_test(test_var_slot_r_simple_disabled1);
    mu_run_test(test_var_slot_r_simple_disabled2);
    mu_run_test(test_var_slot_r_simple_disabled3);
    mu_run_test(test_var_slot_r_simple_direct_cycle);
    mu_run_test(test_var_slot_r_simple_indirect_cycle);
    mu_run_test(test_var_slot_rw_integer);
    mu_run_test(test_var_slot_rw_boolean);
    mu_run_test(test_var_slot_r_simple_def);
    mu_run_test(test_var_slot_r_simple_check_missing);
    mu_run_test(test_var_slot_rw_strong1);
    mu_run_test(test_var_slot_rw_strong2);
    mu_run_test(test_var_slot_rw_weak1);
    mu_run_test(test_var_slot_rw_weak2);
    mu_run_test(test_var_slot_rw_tentative1);
    mu_run_test(test_var_slot_rw_tentative2);
    mu_run_test(test_var_slot_check_none);
    mu_run_test(test_var_slot_check_disabled);
}
