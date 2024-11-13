/*****************************************************************************
 *  @file test_package_file.c
 *  Functions for testing package files.
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
#include <package.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "package.file"

/**
 * Tests a package file (1).
 */
mu_test_begin(test_package_file1)
    struct package_t *package;
    struct package_file_t *file;
    char *s;

    package_init_module();

    package = package_add("ipv6");
    package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, "config/ipv6.txt");

    file = package_get_file(package, PACKAGE_FILE_TYPE_CONFIG);
    mu_assert(file);
    mu_assert(package_file_get_package(file) == package);
    mu_assert_eq_uint(PACKAGE_FILE_TYPE_CONFIG, package_file_get_type(file));
    mu_assert_eq_str("config/ipv6.txt", package_file_get_name(file));

    s = package_file_to_string(file);
    mu_assert_eq_str("configuration file 'config/ipv6.txt' (package ipv6)", s);
    free(s);

    package_fini_module();
mu_test_end()

/**
 * Tests a package file (2).
 */
mu_test_begin(test_package_file2)
    struct package_t *package;
    struct package_file_t *file;
    char *s;

    package_init_module();

    package = package_add("ipv6");
    package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, "check/ipv6.exp");

    file = package_get_file(package, PACKAGE_FILE_TYPE_VARTYPE);
    mu_assert(file);
    mu_assert(package_file_get_package(file) == package);
    mu_assert_eq_uint(PACKAGE_FILE_TYPE_VARTYPE, package_file_get_type(file));
    mu_assert_eq_str("check/ipv6.exp", package_file_get_name(file));

    s = package_file_to_string(file);
    mu_assert_eq_str("variable type file 'check/ipv6.exp' (package ipv6)", s);
    free(s);

    package_fini_module();
mu_test_end()

/**
 * Tests a package file (3).
 */
mu_test_begin(test_package_file3)
    struct package_t *package;
    struct package_file_t *file;
    char *s;

    package_init_module();

    package = package_add("ipv6");
    package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, "check/ipv6.txt");

    file = package_get_file(package, PACKAGE_FILE_TYPE_VARDEF);
    mu_assert(file);
    mu_assert(package_file_get_package(file) == package);
    mu_assert_eq_uint(PACKAGE_FILE_TYPE_VARDEF, package_file_get_type(file));
    mu_assert_eq_str("check/ipv6.txt", package_file_get_name(file));

    s = package_file_to_string(file);
    mu_assert_eq_str("variable definition file 'check/ipv6.txt' (package ipv6)", s);
    free(s);

    package_fini_module();
mu_test_end()

/**
 * Tests a package file (4).
 */
mu_test_begin(test_package_file4)
    struct package_t *package;
    struct package_file_t *file;
    char *s;

    package_init_module();

    package = package_add("ipv6");
    package_set_file(package, PACKAGE_FILE_TYPE_EXTCHECK, "check/ipv6.ext");

    file = package_get_file(package, PACKAGE_FILE_TYPE_EXTCHECK);
    mu_assert(file);
    mu_assert(package_file_get_package(file) == package);
    mu_assert_eq_uint(PACKAGE_FILE_TYPE_EXTCHECK, package_file_get_type(file));
    mu_assert_eq_str("check/ipv6.ext", package_file_get_name(file));

    s = package_file_to_string(file);
    mu_assert_eq_str("extended check script 'check/ipv6.ext' (package ipv6)", s);
    free(s);

    package_fini_module();
mu_test_end()

/**
 * Tests a package file (5).
 */
mu_test_begin(test_package_file5)
    struct package_t *package;
    struct package_file_t *file;
    char *s;

    package_init_module();

    package = package_add("ipv6");
    package_set_file(package, PACKAGE_FILE_TYPE_IMAGE, "opt/ipv6.txt");

    file = package_get_file(package, PACKAGE_FILE_TYPE_IMAGE);
    mu_assert(file);
    mu_assert(package_file_get_package(file) == package);
    mu_assert_eq_uint(PACKAGE_FILE_TYPE_IMAGE, package_file_get_type(file));
    mu_assert_eq_str("opt/ipv6.txt", package_file_get_name(file));

    s = package_file_to_string(file);
    mu_assert_eq_str("image file 'opt/ipv6.txt' (package ipv6)", s);
    free(s);

    package_fini_module();
mu_test_end()

void
test_package_file(void)
{
    mu_run_test(test_package_file1);
    mu_run_test(test_package_file2);
    mu_run_test(test_package_file3);
    mu_run_test(test_package_file4);
    mu_run_test(test_package_file5);
}
