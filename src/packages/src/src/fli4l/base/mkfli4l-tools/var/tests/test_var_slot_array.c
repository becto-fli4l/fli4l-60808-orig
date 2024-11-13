/*****************************************************************************
 *  @file test_var_slot_aray.c
 *  Functions for testing array variable slots.
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

#define MU_TESTSUITE "var.slot.array"

/**
 * Tests writing to and reading from a two-dimensional array variable without
 * holes.
 */
mu_test_begin(test_var_slot_rw_array1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    extra = var_slot_extra_create("abc", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "1", extra, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    extra = var_slot_extra_create("def", NULL);
    mu_assert(var_instance_set_and_check_value(inst, "0", extra, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("def", (char const *) value->extra->data);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    mu_assert_eq_str("abc", (char const *) value->extra->data);
    var_value_destroy(value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("10", value->value);
    mu_assert(!value->extra);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("11", value->value);
    mu_assert(!value->extra);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_N");
    mu_assert(var);
    inst = var_instance_create(var, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_%_N");
    mu_assert(var);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * holes.
 */
mu_test_begin(test_var_slot_rw_array2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    props = var_properties_create();
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "21", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "23", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 0;
    inst->indices[1] = 2;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 3;
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));

    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 2;
    inst->indices[1] = 1;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("21", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("21", value->value);
    var_value_destroy(value);
    inst->indices[0] = 2;
    inst->indices[1] = 2;
    mu_assert(!var_instance_try_get_value(inst));
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 2;
    inst->indices[1] = 3;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("23", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("23", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_N");
    mu_assert(var);
    inst = var_instance_create(var, 0, NULL);
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_%_N");
    mu_assert(var);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst->indices[0] = 2;
    value = var_instance_try_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("4", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with too
 * few indices
 */
mu_test_begin(test_var_slot_rw_array_missing_indices)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    extra = var_slot_extra_create("abc", NULL);
    mu_assert(!var_instance_set_and_check_value(inst, "1", extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_slot_extra_destroy(extra);
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with too
 * many indices
 */
mu_test_begin(test_var_slot_rw_array_excessive_indices)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_slot_extra_t *extra;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var, 3, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    inst->indices[2] = 0;
    extra = var_slot_extra_create("abc", NULL);
    mu_assert(!var_instance_set_and_check_value(inst, "1", extra, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_slot_extra_destroy(extra);
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing correct but unchecked values to a two-dimensional array
 * variable without holes and checking them later.
 */
mu_test_begin(test_var_slot_rw_array_unchecked1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("10", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("11", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_N");
    mu_assert(var);
    inst = var_instance_create(var, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_%_N");
    mu_assert(var);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing incorrect and unchecked values to a two-dimensional array
 * variable without holes and checking them later.
 */
mu_test_begin(test_var_slot_rw_array_unchecked2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "A", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_value(inst, "B", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "AA", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_value(inst, "BB", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("B", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("A", value->value);
    var_value_destroy(value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("BB", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("AA", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_N");
    mu_assert(var);
    inst = var_instance_create(var, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var = var_get("V_%_N");
    mu_assert(var);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from two one-dimensional array variables
 * sharing the array bound variable.
 */
mu_test_begin(test_var_slot_rw_array_multi)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *var_n;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var1 = var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    var2 = var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());
    inst1 = var_instance_create(var1, 1, NULL);
    inst2 = var_instance_create(var2, 1, NULL);

    inst1->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst1, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst2->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst2, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst1->indices[0] = 0;
    value = var_instance_get_value(inst1);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst2->indices[0] = 0;
    mu_assert(!var_instance_get_value(inst2));

    inst1->indices[0] = 1;
    mu_assert(!var_instance_get_value(inst1));
    inst2->indices[0] = 1;
    value = var_instance_get_value(inst2);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    mu_assert(!var_check_values());

    var_instance_destroy(inst2);
    var_instance_destroy(inst1);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading an array bound variable belonging to a one-dimensional array
 * where one of the two conditions fo the array variables is disabled.
 */
mu_test_begin(test_var_slot_rw_array_multi_mixed)
    struct vartype_t *type;
    struct vartype_t *bool_type;
    struct var_properties_t *props;
    struct var_t *var1_cond;
    struct var_t *var2_cond;
    struct var_t *var3_cond;
    struct var_t *var4_cond;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *var_n;
    struct var_instance_t *inst_cond;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    bool_type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();

    var1_cond = var_add(var_get_global_scope(), "X_COND", bool_type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 19), var_properties_create());
    var_write(var_get_name(var1_cond), "no", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    var2_cond = var_add(var_get_global_scope(), "Y_COND", bool_type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 20), var_properties_create());
    var_write(var_get_name(var2_cond), "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    props = var_properties_create();
    inst_cond = var_instance_create(var1_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var3_cond = var_add(var_get_global_scope(), "A_COND", bool_type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 21), props);
    var_write(var_get_name(var3_cond), "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    props = var_properties_create();
    inst_cond = var_instance_create(var2_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var4_cond = var_add(var_get_global_scope(), "B_COND", bool_type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), props);
    var_write(var_get_name(var4_cond), "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    props = var_properties_create();
    inst_cond = var_instance_create(var3_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var1 = var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    props = var_properties_create();
    inst_cond = var_instance_create(var4_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var2 = var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_is_enabled(inst_n));
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst2 = var_instance_create(var2, 1, NULL);

    inst1->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst1, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    inst2->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst2, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    var_instance_destroy(inst2);
    var_instance_destroy(inst1);
    var_instance_destroy(inst_n);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests updating an array bound variable belonging to a one-dimensional array
 * where the array is disabled.
 */
mu_test_begin(test_var_slot_rw_array_multi_disabled)
    struct vartype_t *type;
    struct vartype_t *bool_type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *var_n;
    struct var_instance_t *inst_cond;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *inst_n;
    struct var_value_t *value;
    char const *v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    bool_type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "COND", bool_type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 19), var_properties_create());
    var_write(var_get_name(var_cond), "no", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    props = var_properties_create();
    inst_cond = var_instance_create(var_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var1 = var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    props = var_properties_create();
    inst_cond = var_instance_create(var_cond, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst_cond)));
    var2 = var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    mu_assert(!var_instance_is_enabled(inst_n));
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst2 = var_instance_create(var2, 1, NULL);

    inst1->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst1, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    v = var_read("V_1_A");
    mu_assert(!v);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst2->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst2, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    v = var_read("V_2_B");
    mu_assert(!v);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    mu_assert(var_instance_set_and_check_value(inst_n, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    v = var_read("V_N");
    mu_assert(v);
    mu_assert_eq_str("0", v);
    v = var_read("V_1_A");
    mu_assert(!v);
    v = var_read("V_2_B");
    mu_assert(!v);

    mu_assert(var_instance_set_and_check_value(inst_n, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    v = var_read("V_N");
    mu_assert(v);
    mu_assert_eq_str("0", v);
    v = var_read("V_1_A");
    mu_assert(!v);
    v = var_read("V_2_B");
    mu_assert(!v);

    var_write(var_get_name(var_cond), "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SUPERSEDE, TRUE);
    v = var_read("V_N");
    mu_assert(v);
    mu_assert_eq_str("2", v);
    v = var_read("V_1_A");
    mu_assert(v);
    mu_assert_eq_str("1", v);
    v = var_read("V_2_B");
    mu_assert(v);
    mu_assert_eq_str("2", v);

    var_instance_destroy(inst2);
    var_instance_destroy(inst1);
    var_instance_destroy(inst_n);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from two one-dimensional array variables
 * sharing the array bound variable and mututally extending the array size.
 */
mu_test_begin(test_var_slot_rw_array_multi_extend)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *var_n;
    struct var_properties_t *props;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 23)));
    var1 = var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var1);
    inst1 = var_instance_create(var1, 1, NULL);

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 24)));
    var2 = var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props);
    mu_assert(var2);
    inst2 = var_instance_create(var2, 1, NULL);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);

    inst1->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst1, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst2->indices[0] = 0;
    value = var_instance_get_value(inst2);
    mu_assert(value);
    mu_assert_eq_str("24", value->value);
    var_value_destroy(value);

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    inst2->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst2, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst1->indices[0] = 1;
    value = var_instance_get_value(inst1);
    mu_assert(value);
    mu_assert_eq_str("23", value->value);
    var_value_destroy(value);

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    mu_assert(var_check_values());

    var_instance_destroy(inst_n);
    var_instance_destroy(inst2);
    var_instance_destroy(inst1);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bounds.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bounds1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    /* only V_%_%[1][1] is defined (i.e. indices (0, 0)) */
    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);
    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    /* must fail as it is out of bounds */
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    /* must fail as it is out of bounds */
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    /* must fail as it is out of bounds */
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    /* must fail as it is out of bounds */
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bounds where some values outside user-defined bounds are
 * incorrect.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bounds2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);
    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "X", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_value(inst, "Y", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    var_instance_destroy(inst);

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bounds where some bounds are too large.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bounds3)
    struct vartype_t *type;
    struct var_t *var;
    struct var_properties_t *props;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);
    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("10", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("11", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bounds where some bounds are incorrect (1).
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bounds4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_value(inst_n, "ABC", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    mu_assert(var_instance_set_value(inst_n, "-1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(var_instance_set_and_check_value(inst_n, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst_n, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    mu_assert(var_instance_set_value(inst_n, "-1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(!value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(!value);

    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("10", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("11", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);

    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("-1", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bounds where some bounds are incorrect (2).
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bounds5)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_n;
    struct var_instance_t *inst_n;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    mu_assert(var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props));

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst_n, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_value(inst_n, "ABC", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst_n);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    mu_assert(!var_instance_get_value(inst_n));
    var_instance_destroy(inst_n);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bound variables without default or disabled values.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bound_var1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct location_t *loc;
    struct var_t *var_n1;
    struct var_t *var_n2;
    struct var_t *var_n;

    package_init_module();
    vartype_init_module();
    var_init_module();

    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 21);
    var_n1 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n1);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22);
    var_n2 = var_add(var_get_global_scope(), "V_%_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n2);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    mu_assert(var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));

    var_n = var_get("V_N");
    mu_assert(var_n == var_n1);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->def_value);

    var_n = var_get("V_%_N");
    mu_assert(var_n == var_n2);
    propsNew = var_get_properties(var_n);
    mu_assert(!propsNew->def_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bound variables with sensible default and disabled values.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bound_var2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct location_t *loc;
    struct var_t *var_n1;
    struct var_t *var_n2;
    struct var_t *var_n;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();

    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var_properties_set_disabled_value(props, "0");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 21);
    var_n1 = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n1);

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var_properties_set_disabled_value(props, "0");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22);
    var_n2 = var_add(var_get_global_scope(), "V_%_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n2);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    mu_assert(var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));

    var_n = var_get("V_N");
    mu_assert(var_n == var_n1);
    propsNew = var_get_properties(var_n);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_n = var_get("V_%_N");
    mu_assert(var_n == var_n2);
    propsNew = var_get_properties(var_n);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bound variables with non-zero default values.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bound_var3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct location_t *loc;
    struct var_t *var_n;
    struct var_t *var__n;
    struct var_t *var;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();

    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 1)));
    var_properties_set_disabled_value(props, "0");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 21);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n);

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2)));
    var_properties_set_disabled_value(props, "0");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22);
    var__n = var_add(var_get_global_scope(), "V_%_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var__n);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    mu_assert(var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));

    var = var_get("V_N");
    mu_assert(var == var_n);
    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 1);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    var = var_get("V_%_N");
    mu_assert(var == var__n);
    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 2);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("0", propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a two-dimensional array variable with
 * user-defined bound variables with wrong disabled values.
 */
mu_test_begin(test_var_slot_rw_array_user_defined_bound_var4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct location_t *loc;
    struct var_t *var_n;
    struct var_t *var__n;
    struct var_t *var;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();

    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var_properties_set_disabled_value(props, "1");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 21);
    var_n = var_add(var_get_global_scope(), "V_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var_n);

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 0)));
    var_properties_set_disabled_value(props, "2");
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22);
    var__n = var_add(var_get_global_scope(), "V_%_N", vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER), VAR_PRIORITY_CONFIGURATION, loc, props);
    mu_assert(var__n);

    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23);
    mu_assert(var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, loc, props));

    var = var_get("V_N");
    mu_assert(var == var_n);
    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("1", propsNew->dis_value);

    var = var_get("V_%_N");
    mu_assert(var == var__n);
    propsNew = var_get_properties(var);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    mu_assert_eq_str("2", propsNew->dis_value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests checking missing values of an optional array variable.
 */
mu_test_begin(test_var_slot_r_array_check_missing)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    props->optional = TRUE;
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "42", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    inst->indices[0] = 0;
    mu_assert(!var_instance_get_value(inst));
    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a one-dimensional array.
 */
mu_test_begin(test_var_slot_clear1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    props->optional = TRUE;
    var = var_add(var_get_global_scope(), "MATCH_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 1, NULL);

    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "42", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "23", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    var_n = var_get("MATCH_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    inst->num_indices = 0;
    mu_assert(var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 1;
    inst->indices[0] = 0;
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 1;
    mu_assert(!var_instance_get_value(inst));

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a two-dimensional array.
 */
mu_test_begin(test_var_slot_clear2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    props->optional = TRUE;
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    inst->num_indices = 0;
    mu_assert(var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 2;
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(!var_instance_get_value(inst));

    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a subarray of a two-dimensional array.
 */
mu_test_begin(test_var_slot_clear3)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_instance_t *inst_n;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "11", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    inst_n->indices[0] = 1;
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    inst->num_indices = 1;
    inst->indices[0] = 0;
    mu_assert(var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 2;
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("10", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("11", value->value);
    var_value_destroy(value);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 0, NULL);
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst_n = var_instance_create(var_n, 1, NULL);
    inst_n->indices[0] = 0;
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);
    inst_n->indices[0] = 1;
    value = var_instance_get_value(inst_n);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst_n);

    inst->indices[0] = 1;
    inst->indices[0] = 0;
    mu_assert(!var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    mu_assert(var_check_values());

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing an empty subarray of a two-dimensional array with strong
 * access.
 */
mu_test_begin(test_var_slot_clear4)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    var_write("V_N", "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT, TRUE);
    var_write("V_1_N", "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT, TRUE);

    inst->num_indices = 1;
    inst->indices[0] = 0;
    mu_assert(var_instance_clear(inst, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 2;
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(!var_instance_get_value(inst));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(!var_instance_get_value(inst));

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a non-existing subarray of a two-dimensional array.
 */
mu_test_begin(test_var_slot_clear5)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_instance_t *inst2;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_SCRIPT_TRANSIENT, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "abc", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "def", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst2 = var_instance_create(var, 3, NULL);
    inst2->indices[0] = 0;
    inst2->indices[1] = 0;
    inst2->indices[2] = 0;
    mu_assert(!var_instance_clear(inst2, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));
    var_instance_destroy(inst2);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a subarray of a two-dimensional array with insufficient
 * non-tentative access.
 */
mu_test_begin(test_var_slot_clear6)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "abc", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "def", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "ghi", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "jkl", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));

    inst->num_indices = 1;
    inst->indices[0] = 0;
    mu_assert(!var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 2;
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("abc", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("def", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("ghi", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("jkl", value->value);
    var_value_destroy(value);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests clearing a subarray of a two-dimensional array with insufficient
 * tentative access.
 */
mu_test_begin(test_var_slot_clear7)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add(NULL, ".*", "", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    inst = var_instance_create(var, 2, NULL);

    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "abc", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "def", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "ghi", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "jkl", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));

    inst->num_indices = 1;
    inst->indices[0] = 0;
    mu_assert(var_instance_clear(inst, VAR_ACCESS_TENTATIVE, VAR_PRIORITY_SCRIPT_TRANSIENT));

    inst->num_indices = 2;
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("abc", value->value);
    var_value_destroy(value);
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("def", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("ghi", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("jkl", value->value);
    var_value_destroy(value);

    var_instance_destroy(inst);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the creation and manipulation of array bound variables. Two arrays
 * with the same priority share the same array bound and access it with a
 * priority equal to or greater than the base priority of the variables.
 */
mu_test_begin(test_var_slot_array_bound1)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *varn;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *instn;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    var2 = var_add(var_get_global_scope(), "V_%_X", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());
    mu_assert(var2);
    varn = var_get("V_N");
    mu_assert(varn);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(varn));
    instn = var_instance_create(varn, 0, NULL);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst1->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst1, "42", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst1);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    inst2 = var_instance_create(var2, 1, NULL);
    inst2->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst2, "43", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_PERSISTENT));
    var_instance_destroy(inst2);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_int(VAR_PRIORITY_ARRAY_BOUND, value->priority);
    mu_assert_eq_str("3", value->value);
    var_value_destroy(value);

    var_instance_destroy(instn);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the creation and manipulation of array bound variables. Two arrays
 * with the same priority share the same array bound and access it with a
 * priority equal to or less than the base priority of the variables.
 */
mu_test_begin(test_var_slot_array_bound2)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *varn;
    struct var_instance_t *inst1;
    struct var_instance_t *inst2;
    struct var_instance_t *instn;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    var2 = var_add(var_get_global_scope(), "V_%_X", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());
    mu_assert(var2);
    varn = var_get("V_N");
    mu_assert(varn);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(varn));
    instn = var_instance_create(varn, 0, NULL);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst1->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst1, "42", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst1);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    inst2 = var_instance_create(var2, 1, NULL);
    inst2->indices[0] = 2;
    mu_assert(!var_instance_set_and_check_value(inst2, "43", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_SCRIPT_TRANSIENT));
    var_instance_destroy(inst2);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_int(VAR_PRIORITY_ARRAY_BOUND, value->priority);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    var_instance_destroy(instn);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the creation and manipulation of array bound variables. Two arrays
 * with different priorities try to share the same array bound (1).
 */
mu_test_begin(test_var_slot_array_bound3)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *varn;
    struct var_properties_t *props;
    struct location_t *loc;
    struct var_instance_t *inst1;
    struct var_instance_t *instn;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(var_get_global_scope(), "V_%_X", type, VAR_PRIORITY_SCRIPT_TRANSIENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);
    varn = var_get("V_N");
    mu_assert(varn);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(varn));
    instn = var_instance_create(varn, 0, NULL);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst1->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst1, "42", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst1);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    var_instance_destroy(instn);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the creation and manipulation of array bound variables. Two arrays
 * with different priorities try to share the same array bound (2).
 */
mu_test_begin(test_var_slot_array_bound4)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *varn;
    struct var_properties_t *props;
    struct location_t *loc;
    struct var_instance_t *inst1;
    struct var_instance_t *instn;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var1 = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var1);
    props = var_properties_create();
    loc = test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24);
    var2 = var_add(var_get_global_scope(), "V_%_X", type, VAR_PRIORITY_SCRIPT_PERSISTENT, loc, props);
    mu_assert(!var2);
    location_destroy(loc);
    var_properties_destroy(props);
    varn = var_get("V_N");
    mu_assert(varn);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, var_get_priority(varn));
    instn = var_instance_create(varn, 0, NULL);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    var_value_destroy(value);

    inst1 = var_instance_create(var1, 1, NULL);
    inst1->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst1, "42", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst1);

    value = var_instance_get_value(instn);
    mu_assert(value);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);

    var_instance_destroy(instn);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests reading an unset array element which has a default value defined by
 * an element of some other array.
 */
mu_test_begin(test_var_slot_array_crossover_defaults)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *inst;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "V_%_B")));
    var1 = var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    props = var_properties_create();
    var2 = var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props);

    inst = var_instance_create(var2, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var1, 1, NULL);
    inst->indices[0] = 0;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_DEFAULT, value->kind);
    mu_assert_eq_str("1", value->value);
    var_value_destroy(value);
    inst->indices[0] = 1;
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_DEFAULT, value->kind);
    mu_assert_eq_str("2", value->value);
    var_value_destroy(value);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_slot_array(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_slot_rw_array1);
    mu_run_test(test_var_slot_rw_array2);
    mu_run_test(test_var_slot_rw_array_missing_indices);
    mu_run_test(test_var_slot_rw_array_excessive_indices);
    mu_run_test(test_var_slot_rw_array_multi);
    mu_run_test(test_var_slot_rw_array_multi_mixed);
    mu_run_test(test_var_slot_rw_array_multi_disabled);
    mu_run_test(test_var_slot_rw_array_multi_extend);
    mu_run_test(test_var_slot_rw_array_unchecked1);
    mu_run_test(test_var_slot_rw_array_unchecked2);
    mu_run_test(test_var_slot_rw_array_user_defined_bounds1);
    mu_run_test(test_var_slot_rw_array_user_defined_bounds2);
    mu_run_test(test_var_slot_rw_array_user_defined_bounds3);
    mu_run_test(test_var_slot_rw_array_user_defined_bounds4);
    mu_run_test(test_var_slot_rw_array_user_defined_bounds5);
    mu_run_test(test_var_slot_rw_array_user_defined_bound_var1);
    mu_run_test(test_var_slot_rw_array_user_defined_bound_var2);
    mu_run_test(test_var_slot_rw_array_user_defined_bound_var3);
    mu_run_test(test_var_slot_rw_array_user_defined_bound_var4);
    mu_run_test(test_var_slot_r_array_check_missing);
    mu_run_test(test_var_slot_clear1);
    mu_run_test(test_var_slot_clear2);
    mu_run_test(test_var_slot_clear3);
    mu_run_test(test_var_slot_clear4);
    mu_run_test(test_var_slot_clear5);
    mu_run_test(test_var_slot_clear6);
    mu_run_test(test_var_slot_clear7);
    mu_run_test(test_var_slot_array_bound1);
    mu_run_test(test_var_slot_array_bound2);
    mu_run_test(test_var_slot_array_bound3);
    mu_run_test(test_var_slot_array_bound4);
    mu_run_test(test_var_slot_array_crossover_defaults);
}
