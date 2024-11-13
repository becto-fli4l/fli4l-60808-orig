/*****************************************************************************
 *  @file test_cmpfunc.c
 *  Functions for testing compare functions.
 *
 *  Copyright (c) 2015-2016 The fli4l team
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
#include <container/cmpfunc.h>
#include <libmkfli4l/str.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MU_TESTSUITE "container.cmpfunc"

/**
 * Tests comparing string in a case-sensitive way.
 */
mu_test_begin(test_cmpfunc_string)
    char const *val1 = "";
    char const *val2 = "";
    mu_assert_eq_int(TRUE, compare_string_keys(&val1, &val2));

    val1 = "a";
    val2 = "a";
    mu_assert_eq_int(TRUE, compare_string_keys(&val1, &val2));

    val1 = "a";
    val2 = "A";
    mu_assert_eq_int(FALSE, compare_string_keys(&val1, &val2));

    val1 = "Dies ist ein langer Satz ohne Umlaute.";
    val2 = "Dies ist ein langer Satz ohne Umlaute.";
    mu_assert_eq_int(TRUE, compare_string_keys(&val1, &val2));

    val2 = "DIES IST EIN LANGER SATZ OHNE UMLAUTE.";
    mu_assert_eq_int(FALSE, compare_string_keys(&val1, &val2));

    val2 = "Dies ist ein längerer Satz mit Umlauten.";
    mu_assert_eq_int(FALSE, compare_string_keys(&val1, &val2));
mu_test_end()

/**
 * Tests comparing string in a case-insensitive way.
 */
mu_test_begin(test_cmpfunc_ci_string)
    char const *val1 = "";
    char const *val2 = "";
    mu_assert_eq_int(TRUE, compare_ci_string_keys(&val1, &val2));

    val1 = "a";
    val2 = "a";
    mu_assert_eq_int(TRUE, compare_ci_string_keys(&val1, &val2));

    val1 = "a";
    val2 = "A";
    mu_assert_eq_int(TRUE, compare_ci_string_keys(&val1, &val2));

    val1 = "Dies ist ein langer Satz ohne Umlaute.";
    val2 = "Dies ist ein langer Satz ohne Umlaute.";
    mu_assert_eq_int(TRUE, compare_ci_string_keys(&val1, &val2));

    val2 = "DIES IST EIN LANGER SATZ OHNE UMLAUTE.";
    mu_assert_eq_int(TRUE, compare_ci_string_keys(&val1, &val2));

    val2 = "Dies ist ein längerer Satz mit Umlauten.";
    mu_assert_eq_int(FALSE, compare_ci_string_keys(&val1, &val2));

    /* we use C locale */
    val1 = "ä";
    val2 = "Ä";
    mu_assert_eq_int(FALSE, compare_ci_string_keys(&val1, &val2));
mu_test_end()

/**
 * Tests comparing short integers.
 */
mu_test_begin(test_cmpfunc_short)
    short val1 = 0;
    short val2 = 0;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    val1 = SHRT_MIN;
    val2 = SHRT_MIN;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = SHRT_MAX;
    val2 = SHRT_MAX;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = SHRT_MIN;
    val2 = SHRT_MAX;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    /* wrap around */
    val1 = SHRT_MIN;
    val2 = (short) (SHRT_MAX + 1);
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));
mu_test_end()

/**
 * Tests comparing integers.
 */
mu_test_begin(test_cmpfunc_int)
    int val1 = 0;
    int val2 = 0;
    mu_assert_eq_int(TRUE, compare_int_keys(&val1, &val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_int(TRUE, compare_int_keys(&val1, &val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_int(FALSE, compare_int_keys(&val1, &val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_int(FALSE, compare_int_keys(&val1, &val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_int(FALSE, compare_int_keys(&val1, &val2));

    val1 = INT_MIN;
    val2 = INT_MIN;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = INT_MAX;
    val2 = INT_MAX;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = INT_MIN;
    val2 = INT_MAX;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    /* wrap around */
    val1 = INT_MIN;
    val2 = (int) ((unsigned int) INT_MAX + 1);
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));
mu_test_end()

/**
 * Tests comparing long integers.
 */
mu_test_begin(test_cmpfunc_long)
    long val1 = 0;
    long val2 = 0;
    mu_assert_eq_long(TRUE, compare_long_keys(&val1, &val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_long(TRUE, compare_long_keys(&val1, &val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_long(FALSE, compare_long_keys(&val1, &val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_long(FALSE, compare_long_keys(&val1, &val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_long(FALSE, compare_long_keys(&val1, &val2));

    val1 = LONG_MIN;
    val2 = LONG_MIN;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = LONG_MAX;
    val2 = LONG_MAX;
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));

    val1 = LONG_MIN;
    val2 = LONG_MAX;
    mu_assert_eq_int(FALSE, compare_short_keys(&val1, &val2));

    /* wrap around */
    val1 = LONG_MIN;
    val2 = (long) ((unsigned long) LONG_MAX + 1);
    mu_assert_eq_int(TRUE, compare_short_keys(&val1, &val2));
mu_test_end()

void
test_cmpfunc(void)
{
    mu_run_test(test_cmpfunc_string);
    mu_run_test(test_cmpfunc_ci_string);
    mu_run_test(test_cmpfunc_short);
    mu_run_test(test_cmpfunc_int);
    mu_run_test(test_cmpfunc_long);
}
