/*****************************************************************************
 *  @file test_hashfunc.c
 *  Functions for testing hash functions.
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
#include <container/hashfunc.h>
#include <libmkfli4l/str.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MU_TESTSUITE "container.hashfunc"

/**
 * Tests hashing strings in a case-sensitive way. Note that the testcases
 * require a somewhat "good" hash distribution.
 */
mu_test_begin(test_hashfunc_string)
    char const *val1 = "";
    char const *val2 = "";
    mu_assert_eq_int(hash_string(&val1), hash_string(&val2));

    val1 = "a";
    val2 = "a";
    mu_assert_eq_int(hash_string(&val1), hash_string(&val2));

    val1 = "a";
    val2 = "A";
    mu_assert_eq_int(TRUE, hash_string(&val1) != hash_string(&val2));

    val1 = "a";
    val2 = "Dies ist ein langer Satz ohne Umlaute.";
    mu_assert_eq_int(TRUE, hash_string(&val1) != hash_string(&val2));
mu_test_end()

/**
 * Tests hashing string in a case-insensitive way. Note that the testcases
 * require a somewhat "good" hash distribution.
 */
mu_test_begin(test_hashfunc_ci_string)
    char const *val1 = "";
    char const *val2 = "";
    mu_assert_eq_int(hash_ci_string(&val1), hash_ci_string(&val2));

    val1 = "a";
    val2 = "a";
    mu_assert_eq_int(hash_ci_string(&val1), hash_ci_string(&val2));

    val1 = "a";
    val2 = "A";
    mu_assert_eq_int(hash_ci_string(&val1), hash_ci_string(&val2));

    val1 = "a";
    val2 = "Dies ist ein langer Satz ohne Umlaute.";
    mu_assert_eq_int(TRUE, hash_ci_string(&val1) != hash_ci_string(&val2));

    /* we use C locale */
    val1 = "ä";
    val2 = "Ä";
    mu_assert_eq_int(TRUE, hash_ci_string(&val1) != hash_ci_string(&val2));
mu_test_end()

/**
 * Tests hashing short integers. Note that the testcases require a somewhat
 * "good" hash distribution.
 */
mu_test_begin(test_hashfunc_short)
    short val1 = 0;
    short val2 = 0;
    mu_assert_eq_int(hash_short(&val1), hash_short(&val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_int(hash_short(&val1), hash_short(&val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_int(TRUE, hash_short(&val1) != hash_short(&val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_int(TRUE, hash_short(&val1) != hash_short(&val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_int(TRUE, hash_short(&val1) != hash_short(&val2));
mu_test_end()

/**
 * Tests hashing integers. Note that the testcases require a somewhat "good"
 * hash distribution.
 */
mu_test_begin(test_hashfunc_int)
    int val1 = 0;
    int val2 = 0;
    mu_assert_eq_int(hash_int(&val1), hash_int(&val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_int(hash_int(&val1), hash_int(&val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_int(TRUE, hash_int(&val1) != hash_int(&val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_int(TRUE, hash_int(&val1) != hash_int(&val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_int(TRUE, hash_int(&val1) != hash_int(&val2));
mu_test_end()

/**
 * Tests hashing long integers. Note that the testcases require a somewhat
 * "good" hash distribution.
 */
mu_test_begin(test_hashfunc_long)
    long val1 = 0;
    long val2 = 0;
    mu_assert_eq_int(hash_long(&val1), hash_long(&val2));

    val1 = 42;
    val2 = 42;
    mu_assert_eq_int(hash_long(&val1), hash_long(&val2));

    val1 = 23;
    val2 = 42;
    mu_assert_eq_int(TRUE, hash_long(&val1) != hash_long(&val2));

    val1 = 42;
    val2 = 23;
    mu_assert_eq_int(TRUE, hash_long(&val1) != hash_long(&val2));

    val1 = 1;
    val2 = -1;
    mu_assert_eq_int(TRUE, hash_long(&val1) != hash_long(&val2));
mu_test_end()

void
test_hashfunc(void)
{
    mu_run_test(test_hashfunc_string);
    mu_run_test(test_hashfunc_ci_string);
    mu_run_test(test_hashfunc_short);
    mu_run_test(test_hashfunc_int);
    mu_run_test(test_hashfunc_long);
}
