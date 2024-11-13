/*****************************************************************************
 *  @file test_var_wrapper.c
 *  Functions for testing wrapper functions.
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
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.wrapper"

/**
 * Tests writing to and reading from a non-array variable.
 */
mu_test_begin(test_var_wrapper_rw_simple)
    struct vartype_t *type;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    mu_assert(!var_try_read("V"));
    mu_assert(!var_read("V"));

    mu_assert(var_write("V", "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert_eq_str("0", var_try_read("V"));
    mu_assert_eq_str("0", var_read("V"));

    mu_assert(!var_write("V", "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert_eq_str("0", var_try_read("V"));
    mu_assert_eq_str("0", var_read("V"));

    mu_assert(var_write("V", "4", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, FALSE));
    mu_assert_eq_str("4", var_try_read("V"));
    mu_assert_eq_str("4", var_read("V"));

    mu_assert(var_write("V", "0", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from an array variable.
 */
mu_test_begin(test_var_wrapper_rw_array)
    struct vartype_t *type;
    struct var_properties_t *props;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "number expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    props = var_properties_create();
    var_add(var_get_global_scope(), "V_%_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);

    mu_assert(var_write("V_1_2", "1", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert(var_write("V_1_1", "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert(var_write("V_2_2", "11", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert(var_write("V_2_1", "10", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));

    mu_assert_eq_str("1", var_try_read("V_1_2"););
    mu_assert_eq_str("0", var_try_read("V_1_1"));
    mu_assert_eq_str("11", var_try_read("V_2_2"));
    mu_assert_eq_str("10", var_try_read("V_2_1"));

    mu_assert_eq_str("1", var_read("V_1_2"););
    mu_assert_eq_str("0", var_read("V_1_1"));
    mu_assert_eq_str("11", var_read("V_2_2"));
    mu_assert_eq_str("10", var_read("V_2_1"));

    mu_assert_eq_str("2", var_try_read("V_1_N"));
    mu_assert_eq_str("2", var_try_read("V_2_N"));
    mu_assert_eq_str("2", var_read("V_1_N"));
    mu_assert_eq_str("2", var_read("V_2_N"));

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests writing to and reading from a non-existent variable.
 */
mu_test_begin(test_var_wrapper_rw_non_existent)
    package_init_module();
    vartype_init_module();
    var_init_module();

    mu_assert(!var_try_read("V"));
    mu_assert(!var_read("V"));

    mu_assert(!var_write("V", "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    mu_assert(!var_write("V", "0", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE));

    mu_assert(!var_try_read("V"));
    mu_assert(!var_read("V"));

    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_wrapper(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_wrapper_rw_simple);
    mu_run_test(test_var_wrapper_rw_array);
    mu_run_test(test_var_wrapper_rw_non_existent);
}
