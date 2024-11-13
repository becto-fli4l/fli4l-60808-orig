/*****************************************************************************
 *  @file test_var_inst.c
 *  Functions for testing variable instantiations.
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
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.inst"

/**
 * Tests instantiating a non-array variable.
 */
mu_test_begin(test_var_inst_simple)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_is_valid(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V", name);
    free(name);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a one-dimensional array.
 */
mu_test_begin(test_var_inst_array1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[1];
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    indices[0] = 2;
    inst = var_instance_create(var, 1, indices);
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V_3", name);
    free(name);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests instantiating a three-dimensional array.
 */
mu_test_begin(test_var_inst_array2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[3];
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    indices[0] = 2;
    indices[1] = 3;
    indices[2] = 4;
    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V_3_A_4_B_5_C", name);
    free(name);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests partial instantiation of a three-dimensional array with two indices.
 */
mu_test_begin(test_var_inst_array2_partial1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[2];
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    indices[0] = 2;
    indices[1] = 3;
    inst = var_instance_create(var, 2, indices);
    mu_assert(var_instance_is_valid(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V_3_A_4_B_%_C", name);
    free(name);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests partial instantiation of a three-dimensional array with no indices.
 */
mu_test_begin(test_var_inst_array2_partial2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_is_valid(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V_%_A_%_B_%_C", name);
    free(name);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests var_instance_create_from_string() using a simple variable name.
 */
mu_test_begin(test_var_instance_create_from_string1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create_from_string(var_get_global_scope(), "V");
    mu_assert(inst);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(inst->var == var);
    mu_assert(inst->num_indices == 0);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests var_instance_create_from_string() using an instantiated one-dimensional array.
 */
mu_test_begin(test_var_instance_create_from_string2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create_from_string(var_get_global_scope(), "V_1");
    mu_assert(inst);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(inst->var == var);
    mu_assert(inst->num_indices == 1);
    mu_assert_eq_uint(0, inst->indices[0]);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests var_instance_create_from_string() using an instantiated three-dimensional array.
 */
mu_test_begin(test_var_instance_create_from_string3)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    inst = var_instance_create_from_string(var_get_global_scope(), "V_1_A_2_B_4_C");
    mu_assert(inst);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(inst->var == var);
    mu_assert(inst->num_indices == 3);
    mu_assert_eq_uint(0, inst->indices[0]);
    mu_assert_eq_uint(1, inst->indices[1]);
    mu_assert_eq_uint(3, inst->indices[2]);
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests what happens when a non-array variable having instances is destroyed.
 */
mu_test_begin(test_var_instance_destroy_var_non_array)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    char *name;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);
    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(var_instance_is_enabled(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V", name);
    free(name);
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("0", var_read("V"));
    var_value_destroy(value);

    var_remove(var);
    mu_assert(!var_instance_is_valid(inst));
    mu_assert(!var_instance_is_enabled(inst));
    name = var_instance_to_string(inst);
    mu_assert(!name);
    mu_assert(!var_read("V"));

    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests what happens when an array variable having instances is destroyed.
 */
mu_test_begin(test_var_instance_destroy_var_array)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    char *name;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    inst = var_instance_create(var, 1, NULL);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(var_instance_is_enabled(inst));
    name = var_instance_to_string(inst);
    mu_assert_eq_str("V_1", name);
    free(name);
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    value = var_instance_get_value(inst);
    mu_assert(value);
    mu_assert_eq_str("0", value->value);
    mu_assert_eq_str("0", var_read("V_1"));
    var_value_destroy(value);

    var_remove(var);
    mu_assert(!var_instance_is_valid(inst));
    mu_assert(!var_instance_is_enabled(inst));
    name = var_instance_to_string(inst);
    mu_assert(!name);
    mu_assert(!var_read("V_1"));

    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests var_instance_get_next_free_index() using a three-dimensional array.
 */
mu_test_begin(test_var_instance_get_next_free_index)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    unsigned indices[3];

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%_A_%_B_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var);

    indices[0] = 0;
    indices[1] = 0;
    indices[2] = 0;

    inst = var_instance_create(var, 0, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(1, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(1, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(1, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    indices[0] = 1;
    indices[1] = 2;
    indices[2] = 3;
    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(2, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(3, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(4, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 3, indices);
    mu_assert(var_instance_is_valid(inst));
    mu_assert_eq_uint(0, var_instance_get_next_free_index(inst));
    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_inst(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_inst_simple);
    mu_run_test(test_var_inst_array1);
    mu_run_test(test_var_inst_array2);
    mu_run_test(test_var_inst_array2_partial1);
    mu_run_test(test_var_inst_array2_partial2);
    mu_run_test(test_var_instance_create_from_string1);
    mu_run_test(test_var_instance_create_from_string2);
    mu_run_test(test_var_instance_create_from_string3);
    mu_run_test(test_var_instance_destroy_var_non_array);
    mu_run_test(test_var_instance_destroy_var_array);
    mu_run_test(test_var_instance_get_next_free_index);
}
