/*****************************************************************************
 *  @file test_package_core.c
 *  Functions for testing packages.
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

#define MU_TESTSUITE "package.core"

/**
 * Tests adding a package.
 */
mu_test_begin(test_package_add)
    struct package_t *package;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);
    mu_assert_eq_str("ipv6", package_get_name(package));
    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_CONFIG));
    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_VARTYPE));
    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_VARDEF));
    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_EXTCHECK));
    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_IMAGE));

    package_fini_module();
mu_test_end()

/**
 * Tests package redefinition.
 */
mu_test_begin(test_package_add_redef)
    struct package_t *package;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    package = package_add("ipv6");
    mu_assert(!package);

    package_fini_module();
mu_test_end()

/**
 * Tests getting-or-adding a package.
 */
mu_test_begin(test_package_get_or_add)
    struct package_t *package1;
    struct package_t *package2;

    package_init_module();

    mu_assert(!package_try_get("ipv6"));

    package1 = package_get_or_add("ipv6");
    mu_assert(package1);

    package2 = package_get_or_add("ipv6");
    mu_assert(package2);
    mu_assert(package1 == package2);

    package_fini_module();
mu_test_end()

/**
 * Tests setting name of file containing configuration.
 */
mu_test_begin(test_package_set_config)
    struct package_t *package;
    struct package_file_t *file;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_CONFIG));
    package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, "config/ipv6.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_CONFIG);
    mu_assert(file);
    mu_assert_eq_str("config/ipv6.txt", package_file_get_name(file));

    package_fini_module();
mu_test_end()

/**
 * Tests setting name of file containing variable types.
 */
mu_test_begin(test_package_set_vartype)
    struct package_t *package;
    struct package_file_t *file;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_VARTYPE));
    package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, "check/ipv6.exp");
    file = package_get_file(package, PACKAGE_FILE_TYPE_VARTYPE);
    mu_assert(file);
    mu_assert_eq_str("check/ipv6.exp", package_file_get_name(file));

    package_fini_module();
mu_test_end()

/**
 * Tests setting name of file containing variable definitions.
 */
mu_test_begin(test_package_set_vardef)
    struct package_t *package;
    struct package_file_t *file;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_VARDEF));
    package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, "check/ipv6.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_VARDEF);
    mu_assert(file);
    mu_assert_eq_str("check/ipv6.txt", package_file_get_name(file));

    package_fini_module();
mu_test_end()

/**
 * Tests setting name of file containing an extended check script.
 */
mu_test_begin(test_package_set_extcheck)
    struct package_t *package;
    struct package_file_t *file;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_EXTCHECK));
    package_set_file(package, PACKAGE_FILE_TYPE_EXTCHECK, "check/ipv6.ext");
    file = package_get_file(package, PACKAGE_FILE_TYPE_EXTCHECK);
    mu_assert(file);
    mu_assert_eq_str("check/ipv6.ext", package_file_get_name(file));

    package_fini_module();
mu_test_end()

/**
 * Tests setting name of file containing image copy actions.
 */
mu_test_begin(test_package_set_image)
    struct package_t *package;
    struct package_file_t *file;

    package_init_module();

    package = package_add("ipv6");
    mu_assert(package);

    mu_assert(!package_get_file(package, PACKAGE_FILE_TYPE_IMAGE));
    package_set_file(package, PACKAGE_FILE_TYPE_IMAGE, "opt/ipv6.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_IMAGE);
    mu_assert(file);
    mu_assert_eq_str("opt/ipv6.txt", package_file_get_name(file));

    package_fini_module();
mu_test_end()

/**
 * Tests looking up a package.
 */
mu_test_begin(test_package_get1)
    struct package_t *package1;
    struct package_t *package2;

    package_init_module();

    package1 = package_add("ipv6");
    mu_assert(package1);
    package2 = package_try_get("ipv6");
    mu_assert(package1 == package2);
    package2 = package_get("ipv6");
    mu_assert(package1 == package2);

    package_fini_module();
mu_test_end()

/**
 * Tests looking up a non-existing package.
 */
mu_test_begin(test_package_get2)
    package_init_module();

    mu_assert(!package_try_get("ipv6"));
    mu_assert(!package_get("ipv6"));

    package_fini_module();
mu_test_end()

static BOOL
test_package_foreach_visitor1(void *entry, void *data)
{
    ++*(int *) data;
    return TRUE;
    (void) entry;
}

static BOOL
test_package_foreach_visitor2(void *entry, void *data)
{
    return FALSE;
    (void) data;
    (void) entry;
}

/**
 * Tests var_scope_foreach().
 */
mu_test_begin(test_package_foreach)
    int i;

    package_init_module();

    mu_assert(package_add("base"));
    mu_assert(package_add("ipv6"));

    i = 0;
    mu_assert(package_foreach(test_package_foreach_visitor1, &i));
    mu_assert_eq_int(2, i);
    i = 0;
    mu_assert(!package_foreach(test_package_foreach_visitor2, &i));
    mu_assert_eq_int(0, i);

    package_fini_module();
mu_test_end()

void
test_package_core(void)
{
    mu_run_test(test_package_add);
    mu_run_test(test_package_add_redef);
    mu_run_test(test_package_get_or_add);
    mu_run_test(test_package_set_config);
    mu_run_test(test_package_set_vartype);
    mu_run_test(test_package_set_vardef);
    mu_run_test(test_package_set_extcheck);
    mu_run_test(test_package_set_image);
    mu_run_test(test_package_get1);
    mu_run_test(test_package_get2);
    mu_run_test(test_package_foreach);
}
