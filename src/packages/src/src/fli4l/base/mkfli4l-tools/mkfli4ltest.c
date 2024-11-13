/*****************************************************************************
 *  @file mkfli4ltest.c
 *  Test runner for mkfli4l.
 *
 *  Copyright (c) 2012-2016 - fli4l-Team <team@fli4l.de>
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

#include <testing/minunit.h>
#include <testing/tests.h>
#include <libmkfli4l/log.h>
#include <stdio.h>
#include <string.h>

static void
run_all_tests(void)
{
    test_cmpfunc();
    test_hashfunc();
    test_hashmap();
    test_expr();
    test_expr_literal();
    test_expr_literal_error();
    test_expr_literal_string();
    test_expr_literal_integer();
    test_expr_literal_boolean();
    test_expr_logical_not();
    test_expr_logical_and();
    test_expr_logical_or();
    test_expr_variable();
    test_expr_equal();
    test_expr_unequal();
    test_expr_match();
    test_expr_tobool();
    test_expr_visitor();
    test_expr_referencer();
    test_expr_typechecker();
    test_package_core();
    test_package_file();
    test_package_location();
    test_vartype_core();
    test_vartype_extension();
    test_vartype_predef();
    test_var_props();
    test_var_extra();
    test_var_core();
    test_var_condition();
    test_var_inst();
    test_var_slot_non_array();
    test_var_slot_array();
    test_var_slot_extra();
    test_var_iter();
    test_var_scope();
    test_var_wrapper();
    test_var_template();
    test_var_part();
    test_vartype_scanner();
    test_vartype_parser();
    test_vardef_scanner();
    test_vardef_parser();
    test_varass_scanner();
    test_varass_parser();
    test_expr_scanner();
    test_expr_parser();
}

int
main(int argc, char *argv[])
{
    if (argc >= 3 && strcmp(argv[1], "--output-dir") == 0) {
        mu_set_report_dir(argv[2]);
    }

    no_reformat = TRUE;
    run_all_tests();
    mu_test_finish();

    fprintf(stderr, "\n\n%d tests run: %d successful, %d failed.\n",
            mu_num_tests_run, mu_num_tests_run - mu_num_tests_failed, mu_num_tests_failed);
    return mu_num_tests_failed != 0 ? 1 : 0;
}
