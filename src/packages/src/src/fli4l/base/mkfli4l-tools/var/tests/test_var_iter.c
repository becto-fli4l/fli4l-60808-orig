/*****************************************************************************
 *  @file test_var_iter.c
 *  Functions for testing iteration of arrays.
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

#define MU_TESTSUITE "var.iter"

/**
 * Tests iterating over a one-dimensional array without holes.
 */
mu_test_begin(test_var_iter_onedim_no_holes)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array without holes (all values).
 */
mu_test_begin(test_var_iter_onedim_no_holes_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with holes.
 */
mu_test_begin(test_var_iter_onedim_holes)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 4;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("5", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(4, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with holes (all values).
 */
mu_test_begin(test_var_iter_onedim_holes_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 4;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("5", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(4, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with disabled slots.
 */
mu_test_begin(test_var_iter_onedim_disabled)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with disabled slots (all
 * values).
 */
mu_test_begin(test_var_iter_onedim_disabled_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with disabled slots where the
 * variable has a disabled value.
 */
mu_test_begin(test_var_iter_onedim_disabled_with_value)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var_properties_set_disabled_value(props, "42");
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with disabled slots where the
 * variable has a disabled value (all values).
 */
mu_test_begin(test_var_iter_onedim_disabled_with_value_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_disabled_value(props, "42");
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DISABLED, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with unset slots where the
 * variable has a default value.
 */
mu_test_begin(test_var_iter_onedim_default_value)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DEFAULT, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with unset slots where the
 * variable has a default value (all values).
 */
mu_test_begin(test_var_iter_onedim_default_value_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_DEFAULT, value->kind);
        mu_assert_eq_str("42", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with user-defined bounds.
 */
mu_test_begin(test_var_iter_onedim_user_defined_bounds)
    struct vartype_t *type;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array with user-defined bounds (all
 * values).
 */
mu_test_begin(test_var_iter_onedim_user_defined_bounds_all)
    struct vartype_t *type;
    struct var_t *var;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array without holes.
 */
mu_test_begin(test_var_iter_twodim_no_holes)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("5", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array without holes (all values).
 */
mu_test_begin(test_var_iter_twodim_no_holes_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("5", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with holes.
 */
mu_test_begin(test_var_iter_twodim_holes)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    inst->indices[1] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(3, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with holes (all values).
 */
mu_test_begin(test_var_iter_twodim_holes_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    inst->indices[1] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(2, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(3, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with disabled slots.
 */
mu_test_begin(test_var_iter_twodim_disabled)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 0;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "6", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "7", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "8", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 4;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "9", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 4;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("7", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("8", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with disabled slots (all
 * values).
 */
mu_test_begin(test_var_iter_twodim_disabled_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var_cond;
    struct var_t *var;
    struct var_instance_t *inst;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();

    var_cond = var_add(var_get_global_scope(), "V_%_COND", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 22), var_properties_create());
    props = var_properties_create();
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var_cond, 0, NULL))));
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    inst = var_instance_create(var_cond, 1, NULL);
    inst->indices[0] = 0;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    var_instance_set_and_check_value(inst, "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    var_instance_set_and_check_value(inst, "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 0;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 2;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "6", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "7", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 3;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "8", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 4;
    inst->indices[1] = 0;
    var_instance_set_and_check_value(inst, "9", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    inst->indices[0] = 4;
    inst->indices[1] = 2;
    var_instance_set_and_check_value(inst, "10", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("7", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
        mu_assert(!value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("8", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(3, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with user-defined bounds.
 */
mu_test_begin(test_var_iter_twodim_user_defined_bounds)
    struct vartype_t *type;
    struct var_t *var;
    struct var_t *var_n;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), var_properties_create());

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a two-dimensional array with user-defined bounds (all
 * values).
 */
mu_test_begin(test_var_iter_twodim_user_defined_bounds_all)
    struct vartype_t *type;
    struct var_t *var;
    struct var_t *var_n;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), var_properties_create());

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    var_n = var_get("V_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    var_n = var_get("V_%_N");
    mu_assert(var_n);
    inst = var_instance_create(var_n, 1, NULL);
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("0", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(0, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array which is modified during the
 * iteration.
 */
mu_test_begin(test_var_iter_onedim_modified)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("1", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(0, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("2", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(1, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("3", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(2, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("4", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(3, ind[0]);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 5;
    mu_assert(var_instance_set_and_check_value(inst, "6", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("6", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(5, ind[0]);

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array which is modified during the
 * iteration (all values).
 */
mu_test_begin(test_var_iter_onedim_modified_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("1", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(0, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("2", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(1, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("3", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(2, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("4", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(3, ind[0]);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 5;
    mu_assert(var_instance_set_and_check_value(inst, "6", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NONE, value->kind);
    mu_assert(!value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(4, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("6", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(5, ind[0]);

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over an empty one-dimensional array.
 */
mu_test_begin(test_var_iter_onedim_empty)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(!var_value_iterator_get_var_indices(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over an empty one-dimensional array (all values).
 */
mu_test_begin(test_var_iter_onedim_empty_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(!var_value_iterator_get_var_indices(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over an existing subarray of a two-dimensional array.
 */
mu_test_begin(test_var_iter_subarray1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 1;
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over an existing subarray of a two-dimensional array (all
 * values).
 */
mu_test_begin(test_var_iter_subarray1_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 1;
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("2", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(0, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("3", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(1, ind[1]);

        mu_assert(var_value_iterator_next(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("4", value->value);
        ind = var_value_iterator_get_var_indices(iter);
        mu_assert(ind);
        mu_assert_eq_int(1, ind[0]);
        mu_assert_eq_int(2, ind[1]);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a non-existing subarray of a two-dimensional array.
 */
mu_test_begin(test_var_iter_subarray2)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 3;
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(!var_value_iterator_get_var_indices(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a non-existing subarray of a two-dimensional array (all
 * values).
 */
mu_test_begin(test_var_iter_subarray2_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 2, NULL);
    inst->indices[0] = 0;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 0;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    inst->indices[1] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    inst->indices[1] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "5", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 3;
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get_var_indices(iter));
    mu_assert(!var_value_iterator_get_var_instance(iter));
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(!var_value_iterator_next(iter));
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get_var_indices(iter));
    mu_assert(!var_value_iterator_get_var_instance(iter));
    mu_assert(!var_value_iterator_get(iter));
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a non-array variable.
 */
mu_test_begin(test_var_iter_non_array)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;
    struct var_instance_t *inst_iter;
    struct var_value_t *value;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    inst_iter = var_value_iterator_get_var_instance(iter);
    mu_assert(!inst_iter);

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get_var_indices(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
        inst_iter = var_value_iterator_get_var_instance(iter);
        mu_assert(inst_iter);
        mu_assert_eq_int(0, inst_iter->num_indices);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a non-array variable (all values).
 */
mu_test_begin(test_var_iter_non_array_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;
    struct var_instance_t *inst_iter;
    struct var_value_t *value;
    int i;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 0, NULL);
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    inst_iter = var_value_iterator_get_var_instance(iter);
    mu_assert(!inst_iter);

    for (i = 0; i < 2; ++i) {
        mu_assert(var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get_var_indices(iter));
        value = var_value_iterator_get(iter);
        mu_assert(value);
        mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
        mu_assert_eq_str("1", value->value);
        mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, value->priority);
        inst_iter = var_value_iterator_get_var_instance(iter);
        mu_assert(inst_iter);
        mu_assert_eq_int(0, inst_iter->num_indices);

        mu_assert(!var_value_iterator_next(iter));
        mu_assert(!var_value_iterator_get(iter));
    }

    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests destroying an unfinished iterator.
 */
mu_test_begin(test_var_iter_destroy)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("1", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(0, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("2", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(1, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    var_value_iterator_destroy(iter);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests destroying an unfinished iterator (all values).
 */
mu_test_begin(test_var_iter_destroy_all)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_t *value;
    struct var_value_iterator_t *iter;
    unsigned *ind;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 2;
    mu_assert(var_instance_set_and_check_value(inst, "3", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 3;
    mu_assert(var_instance_set_and_check_value(inst, "4", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, TRUE);
    var_instance_destroy(inst);
    mu_assert(iter);
    mu_assert(var_value_iterator_get_var(iter) == var);
    mu_assert(!var_value_iterator_get(iter));

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("1", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(0, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    value = var_value_iterator_get(iter);
    mu_assert(value);
    mu_assert_eq_int(VAR_VALUE_KIND_NORMAL, value->kind);
    mu_assert_eq_str("2", value->value);
    ind = var_value_iterator_get_var_indices(iter);
    mu_assert(ind);
    mu_assert_eq_int(1, ind[0]);

    mu_assert(var_value_iterator_next(iter));
    var_value_iterator_destroy(iter);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests iterating over a one-dimensional array without holes.
 */
mu_test_begin(test_var_iter_instance)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_t *var;
    struct var_value_iterator_t *iter;
    struct var_instance_t *inst;
    struct var_instance_t *inst_iter;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    vartype_finalize_type_system();
    props = var_properties_create();
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 42), props);

    inst = var_instance_create(var, 1, NULL);
    inst->indices[0] = 0;
    mu_assert(var_instance_set_and_check_value(inst, "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    inst->indices[0] = 1;
    mu_assert(var_instance_set_and_check_value(inst, "2", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    var_instance_destroy(inst);

    inst = var_instance_create(var, 0, NULL);
    iter = var_value_iterator_create(inst, FALSE);
    mu_assert(iter);

    inst_iter = var_value_iterator_get_var_instance(iter);
    mu_assert(!inst_iter);

    mu_assert(var_value_iterator_next(iter));
    inst_iter = var_value_iterator_get_var_instance(iter);
    mu_assert(inst->var == inst_iter->var);
    mu_assert_eq_int(1, inst_iter->num_indices);
    mu_assert_eq_int(0, inst_iter->indices[0]);

    mu_assert(var_value_iterator_next(iter));
    inst_iter = var_value_iterator_get_var_instance(iter);
    mu_assert(inst->var == inst_iter->var);
    mu_assert_eq_int(1, inst_iter->num_indices);
    mu_assert_eq_int(1, inst_iter->indices[0]);

    var_instance_destroy(inst);
    var_value_iterator_destroy(iter);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_iter(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_iter_onedim_no_holes);
    mu_run_test(test_var_iter_onedim_no_holes_all);
    mu_run_test(test_var_iter_onedim_holes);
    mu_run_test(test_var_iter_onedim_holes_all);
    mu_run_test(test_var_iter_onedim_disabled);
    mu_run_test(test_var_iter_onedim_disabled_all);
    mu_run_test(test_var_iter_onedim_disabled_with_value);
    mu_run_test(test_var_iter_onedim_disabled_with_value_all);
    mu_run_test(test_var_iter_onedim_default_value);
    mu_run_test(test_var_iter_onedim_default_value_all);
    mu_run_test(test_var_iter_onedim_user_defined_bounds);
    mu_run_test(test_var_iter_onedim_user_defined_bounds_all);
    mu_run_test(test_var_iter_twodim_no_holes);
    mu_run_test(test_var_iter_twodim_no_holes_all);
    mu_run_test(test_var_iter_twodim_holes);
    mu_run_test(test_var_iter_twodim_holes_all);
    mu_run_test(test_var_iter_twodim_disabled);
    mu_run_test(test_var_iter_twodim_disabled_all);
    mu_run_test(test_var_iter_twodim_user_defined_bounds);
    mu_run_test(test_var_iter_twodim_user_defined_bounds_all);
    mu_run_test(test_var_iter_onedim_modified);
    mu_run_test(test_var_iter_onedim_modified_all);
    mu_run_test(test_var_iter_onedim_empty);
    mu_run_test(test_var_iter_onedim_empty_all);
    mu_run_test(test_var_iter_subarray1);
    mu_run_test(test_var_iter_subarray1_all);
    mu_run_test(test_var_iter_subarray2);
    mu_run_test(test_var_iter_subarray2_all);
    mu_run_test(test_var_iter_non_array);
    mu_run_test(test_var_iter_non_array_all);
    mu_run_test(test_var_iter_destroy);
    mu_run_test(test_var_iter_destroy_all);
    mu_run_test(test_var_iter_instance);
}
