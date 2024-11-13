/*****************************************************************************
 *  @file minunit.h
 *  Functions for unit testing.
 *
 *  This code is based on MinUnit, see:
 *  http://www.jera.com/techinfo/jtns/jtn002.html
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

#include <libmkfli4l/defs.h>
#include <stdio.h>
#include <string.h>

/**
 * Begins a test function.
 * @param func
 *  The name of the test function.
 */
#define mu_test_begin(func) \
    static BOOL func(void) \
    { \
        char const *const mu_function = #func;

/**
 * Ends a test function.
 */
#define mu_test_end() \
        (void) mu_function; \
        return TRUE; \
    }

/**
 * Checks an assertion. If it fails, a message is logged to stderr,
 * mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param test
 *  An expression evaluating to TRUE or FALSE.
 */
#define mu_assert(test) \
    do { \
        if (!(test)) { \
            fprintf(mu_stream, "mu_assert: %s is false\nat %s.%s(%s:%d)", \
                #test, MU_TESTSUITE, mu_function, __FILE__, __LINE__); \
            return FALSE; \
        } \
    } while (0)

/**
 * Checks the equality of numbers. If it fails, a message is logged to stderr,
 * mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 * @param type
 *  The type of the values to be compared.
 * @param fmt
 *  The printf() format string used to display the numbers.
 */
#define mu_assert_eq_num(expected, actual, type, fmt) \
    do { \
        type const exp = (type) (expected); \
        type const act = (type) (actual); \
        if (exp != act) { \
            fprintf(mu_stream, "mu_assert_eq_num: expected " fmt ", got " fmt "\nat %s.%s(%s:%d)", \
                exp, act, MU_TESTSUITE, mu_function, __FILE__, __LINE__); \
            return FALSE; \
        } \
    } while (0)

/**
 * Checks the equality of short integers. If it fails, a message is logged to
 * stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_short(expected, actual) \
    mu_assert_eq_num(expected, actual, short, "%hd")

/**
 * Checks the equality of unsigned short integers. If it fails, a message is
 * logged to stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_ushort(expected, actual) \
    mu_assert_eq_num(expected, actual, unsigned short, "%hu")

/**
 * Checks the equality of integers. If it fails, a message is logged to stderr,
 * mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_int(expected, actual) \
    mu_assert_eq_num(expected, actual, int, "%d")

/**
 * Checks the equality of unsigned integers. If it fails, a message is logged
 * to stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_uint(expected, actual) \
    mu_assert_eq_num(expected, actual, unsigned, "%u")

/**
 * Checks the equality of long integers. If it fails, a message is logged to
 * stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_long(expected, actual) \
    mu_assert_eq_num(expected, actual, long, "%ld")

/**
 * Checks the equality of unsigned long integers. If it fails, a message is
 * logged to stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_ulong(expected, actual) \
    mu_assert_eq_num(expected, actual, unsigned long, "%lu")

/**
 * Checks the equality of strings via strcmp(). If it fails, a message is
 * logged to stderr, mu_num_tests_failed is incremented, and FALSE is returned.
 *
 * @param expected
 *  The expected value.
 * @param actual
 *  The actual value.
 */
#define mu_assert_eq_str(expected, actual) \
    do { \
        char const *exp = expected; \
        char const *act = actual; \
        if (strcmp(exp, act) != 0) { \
            fprintf(mu_stream, "mu_assert_eq_str: expected \"%s\", got \"%s\"\nat %s.%s(%s:%d)", \
                exp, act, MU_TESTSUITE, mu_function, __FILE__, __LINE__); \
            return FALSE; \
        } \
    } while (0)

/**
 * Runs a test function. If the test function succeeds, a dot is printed to
 * stderr, otherwise an F (for "failed"). In both cases, mu_num_tests_run is
 * incremented.
 *
 * @param test
 *  The test function to execute.
 */
#define mu_run_test(test) mu_schedule_test(MU_TESTSUITE, #test, test)

/**
 * Number of tests run.
 */
extern int mu_num_tests_run;

/**
 * Number of failed tests.
 */
extern int mu_num_tests_failed;

/**
 * File handle to write "assertion failed" messages to.
 */
extern FILE *mu_stream;

/**
 * Sets the directory to write the test reports to.
 *
 * @param dir
 *  The directory to write the test reports to.
 */
extern void mu_set_report_dir(char const *dir);

/**
 * Schedules the execution of a single test.
 *
 * @param testsuite
 *  The name of the test suite.
 * @param testcase
 *  The name of the test function to run.
 * @param func
 *  The test function to run.
 */
extern void mu_schedule_test(char const *testsuite, char const *testcase, BOOL (*func)(void));

/**
 * Finishes testing. To be called after the exeuction of the last test case.
 */
extern void mu_test_finish(void);
