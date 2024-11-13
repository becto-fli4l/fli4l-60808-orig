/*****************************************************************************
 *  @file test_package_location.c
 *  Functions for testing locations.
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
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "package.location"

/**
 * Tests location_equals() on regular locations.
 */
mu_test_begin(test_package_location_equals1)
    struct package_t *package1;
    struct package_t *package2;
    struct package_file_t *file1;
    struct package_file_t *file2;
    struct location_t *location1;
    struct location_t *location2;

    package_init_module();
    package1 = package_add("myfile1");
    package_set_file(package1, PACKAGE_FILE_TYPE_CONFIG, "config/myfile1.txt");
    file1 = package_get_file(package1, PACKAGE_FILE_TYPE_CONFIG);
    package2 = package_add("myfile2");
    package_set_file(package2, PACKAGE_FILE_TYPE_CONFIG, "config/myfile2.txt");
    file2 = package_get_file(package2, PACKAGE_FILE_TYPE_CONFIG);
    location1 = location_create(file1, 42, 3);

    location2 = location_create(file1, 42, 3);
    mu_assert(location_equals(location1, location2));
    mu_assert(location_equals(location2, location1));
    location_destroy(location2);

    location2 = location_create(file1, 42, 4);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location2 = location_create(file1, 41, 3);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location2 = location_create(file2, 42, 3);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location2 = location_create(file2, 41, 4);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location_destroy(location1);
    package_fini_module();
mu_test_end()

/**
 * Tests location_equals() on internal locations.
 */
mu_test_begin(test_package_location_equals2)
    struct location_t *location1;
    struct location_t *location2;

    package_init_module();
    location1 = location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK);

    location2 = location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK);
    mu_assert(location_equals(location1, location2));
    mu_assert(location_equals(location2, location1));
    location_destroy(location2);

    location2 = location_create_internal(PACKAGE_FILE_TYPE_IMAGE);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location_destroy(location1);
    package_fini_module();
mu_test_end()

/**
 * Tests location_equals() on regular and internal locations (mixed).
 */
mu_test_begin(test_package_location_equals3)
    struct package_t *package;
    struct package_file_t *file;
    struct location_t *location1;
    struct location_t *location2;

    package_init_module();
    package = package_add("myfile1");
    package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, "config/myfile.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_CONFIG);
    location1 = location_create(file, 42, 3);

    location2 = location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK);
    mu_assert(!location_equals(location1, location2));
    mu_assert(!location_equals(location2, location1));
    location_destroy(location2);

    location_destroy(location1);
    package_fini_module();
mu_test_end()

/**
 * Tests location_toString() on a regular location.
 */
mu_test_begin(test_package_location_toString)
    char *value;
    struct package_t *package;
    struct package_file_t *file;
    struct location_t *location;
    char *locstr;
    char *s;

    package_init_module();
    package = package_add("myfile");
    package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, "config/myfile.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_CONFIG);
    location = location_create(file, 42, 3);
    value = location_toString(location);
    locstr = package_file_to_string(file);
    s = safe_sprintf("[%s 42:3]", locstr);
    free(locstr);
    mu_assert_eq_str(s, value);
    free(s);
    free(value);
    location_destroy(location);
    package_fini_module();
mu_test_end()

/**
 * Tests location_toString() on an internal location.
 */
mu_test_begin(test_package_location_internal_toString)
    char *value;
    struct location_t *location;
    char *locstr;
    char *s;

    package_init_module();
    location = location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK);
    value = location_toString(location);
    locstr = package_file_to_string(package_get_file(package_get_internal(),
            PACKAGE_FILE_TYPE_EXTCHECK));
    s = safe_sprintf("[%s 0:0]", locstr);
    free(locstr);
    mu_assert_eq_str(s, value);
    free(s);
    free(value);
    location_destroy(location);
    package_fini_module();
mu_test_end()

/**
 * Tests location_get_package_file() on a regular location.
 */
mu_test_begin(test_package_location_get_package_file)
    struct package_t *package;
    struct package_file_t *file;
    struct location_t *location;

    package_init_module();
    package = package_add("myfile");
    package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, "config/myfile.txt");
    file = package_get_file(package, PACKAGE_FILE_TYPE_CONFIG);
    location = location_create(file, 42, 3);

    mu_assert(file == location_get_package_file(location));

    location_destroy(location);
    package_fini_module();
mu_test_end()

/**
 * Tests location_get_package_file() on an internal location.
 */
mu_test_begin(test_package_location_internal_get_package_file)
    struct package_file_t *file;
    struct location_t *location;

    package_init_module();
    location = location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK);
    file = package_get_file(package_get_internal(), PACKAGE_FILE_TYPE_EXTCHECK);

    mu_assert(file == location_get_package_file(location));

    location_destroy(location);
    package_fini_module();
mu_test_end()

void
test_package_location(void)
{
    mu_run_test(test_package_location_equals1);
    mu_run_test(test_package_location_equals2);
    mu_run_test(test_package_location_equals3);
    mu_run_test(test_package_location_toString);
    mu_run_test(test_package_location_internal_toString);
    mu_run_test(test_package_location_get_package_file);
    mu_run_test(test_package_location_internal_get_package_file);
}
