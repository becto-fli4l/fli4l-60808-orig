/*****************************************************************************
 *  @file test_vartype_predef.c
 *  Functions for testing predefined variable types.
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
#include <vartype.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "vartype.predef"

/**
 * Tests predefined type VARTYPE_PREDEFINED_ANYTHING.
 */
mu_test_begin(test_vartype_predef_none)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
    mu_assert(type);
    regex = vartype_get_regex(type, FALSE);
    mu_assert_eq_str(".*", regex);
    free(regex);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_ANYTHING with a
 * compatible definition.
 */
mu_test_begin(test_vartype_predef_none_overriding1)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();
    type = vartype_add(VARTYPE_PREDEFINED_ANYTHING, ".*", "", "",
            location_create_internal(PACKAGE_FILE_TYPE_VARTYPE));
    mu_assert(type);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_ANYTHING with an
 * incompatible definition.
 */
mu_test_begin(test_vartype_predef_none_overriding2)
    struct vartype_t *type;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    loc = location_create_internal(PACKAGE_FILE_TYPE_VARTYPE);
    type = vartype_add(VARTYPE_PREDEFINED_ANYTHING, "x.*y", "", "", loc);
    mu_assert(!type);
    location_destroy(loc);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests predefined type VARTYPE_PREDEFINED_UNSIGNED_INTEGER.
 */
mu_test_begin(test_vartype_predef_unsigned_integer)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    mu_assert(type);
    regex = vartype_get_regex(type, FALSE);
    mu_assert_eq_str("0|[1-9][0-9]*", regex);
    free(regex);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_UNSIGNED_INTEGER with a
 * compatible definition.
 */
mu_test_begin(test_vartype_predef_unsigned_integer_overriding1)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();
    type = vartype_add(VARTYPE_PREDEFINED_UNSIGNED_INTEGER, "0|[1-9][0-9]*", "",
            "", location_create_internal(PACKAGE_FILE_TYPE_VARTYPE));
    mu_assert(type);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_UNSIGNED_INTEGER with an
 * incompatible definition.
 */
mu_test_begin(test_vartype_predef_unsigned_integer_overriding2)
    struct vartype_t *type;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    loc = location_create_internal(PACKAGE_FILE_TYPE_VARTYPE);
    type = vartype_add(VARTYPE_PREDEFINED_UNSIGNED_INTEGER, "[0-9]+", "", "",
            loc);
    mu_assert(!type);
    location_destroy(loc);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests predefined type VARTYPE_PREDEFINED_BOOLEAN.
 */
mu_test_begin(test_vartype_predef_yesno)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    mu_assert(type);
    regex = vartype_get_regex(type, FALSE);
    mu_assert_eq_str("yes|no", regex);
    free(regex);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_BOOLEAN with a
 * compatible definition.
 */
mu_test_begin(test_vartype_predef_yesno_overriding1)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();
    type = vartype_add(VARTYPE_PREDEFINED_BOOLEAN, "yes|no", "", "",
            location_create_internal(PACKAGE_FILE_TYPE_VARTYPE));
    mu_assert(type);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests overriding predefined type VARTYPE_PREDEFINED_BOOLEAN with an
 * incompatible definition.
 */
mu_test_begin(test_vartype_predef_yesno_overriding2)
    struct vartype_t *type;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();
    loc = location_create_internal(PACKAGE_FILE_TYPE_VARTYPE);
    type = vartype_add(VARTYPE_PREDEFINED_BOOLEAN, "yes|no|maybe", "", "",
            loc);
    mu_assert(!type);
    location_destroy(loc);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_vartype_predef(void)
{
    set_log_level(LOG_EXP);
    mu_run_test(test_vartype_predef_none);
    mu_run_test(test_vartype_predef_none_overriding1);
    mu_run_test(test_vartype_predef_none_overriding2);
    mu_run_test(test_vartype_predef_unsigned_integer);
    mu_run_test(test_vartype_predef_unsigned_integer_overriding1);
    mu_run_test(test_vartype_predef_unsigned_integer_overriding2);
    mu_run_test(test_vartype_predef_yesno);
    mu_run_test(test_vartype_predef_yesno_overriding1);
    mu_run_test(test_vartype_predef_yesno_overriding2);
}
