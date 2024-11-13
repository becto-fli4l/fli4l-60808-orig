/*****************************************************************************
 *  @file test_var_extra.c
 *  Functions for testing a variable's extra data.
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
#include <var.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>

#define MU_TESTSUITE "var.extra"

static int my_clone_calls = 0;
static int my_destr_calls = 0;

static void *
my_clone(void *data)
{
    ++my_clone_calls;
    return data;
}

static void *
my_str_clone(void *data)
{
    ++my_clone_calls;
    return strsave((char *) data);
}

static void
my_destructor(void *data)
{
    ++my_destr_calls;
    UNUSED(data);
}

/**
 * Tests creating and destroying without value, clone operation, and destructor.
 */
mu_test_begin(test_var_extra_ctor_dtor1)
    struct var_extra_t *extra = var_extra_create(NULL, NULL, NULL);

    mu_assert(!extra->data);
    mu_assert(!extra->clone);
    mu_assert(!extra->free);

    var_extra_destroy(extra);
mu_test_end()

/**
 * Tests creating and destroying with value but without clone operation and
 * destructor.
 */
mu_test_begin(test_var_extra_ctor_dtor2)
    struct var_extra_t *extra = var_extra_create("abc", NULL, NULL);

    mu_assert_eq_str("abc", extra->data);
    mu_assert(!extra->clone);
    mu_assert(!extra->free);

    var_extra_destroy(extra);
mu_test_end()

/**
 * Tests creating and destroying with value, clone operation, and destructor.
 */
mu_test_begin(test_var_extra_ctor_dtor3)
    struct var_extra_t *extra
        = var_extra_create("abc", my_clone, my_destructor);

    mu_assert_eq_str("abc", extra->data);
    mu_assert(extra->clone == my_clone);
    mu_assert(extra->free == my_destructor);

    mu_assert_eq_int(0, my_destr_calls);
    var_extra_destroy(extra);
    mu_assert_eq_int(1, my_destr_calls);
mu_test_end()

/**
 * Tests cloning without value, clone operation, and destructor.
 */
mu_test_begin(test_var_extra_clone1)
    struct var_extra_t *extra = var_extra_create(NULL, NULL, NULL);
    struct var_extra_t *extra2;

    mu_assert(!extra->data);
    mu_assert(!extra->clone);
    mu_assert(!extra->free);

    extra2 = var_extra_clone(extra);
    mu_assert(!extra2->data);
    mu_assert(!extra2->clone);
    mu_assert(!extra2->free);

    var_extra_destroy(extra2);
    var_extra_destroy(extra);
mu_test_end()

/**
 * Tests cloning with value but without clone operation and destructor.
 */
mu_test_begin(test_var_extra_clone2)
    struct var_extra_t *extra = var_extra_create("abc", NULL, NULL);
    struct var_extra_t *extra2;

    mu_assert_eq_str("abc", extra->data);
    mu_assert(!extra->clone);
    mu_assert(!extra->free);

    extra2 = var_extra_clone(extra);
    mu_assert(extra2->data == extra->data);
    mu_assert(!extra2->clone);
    mu_assert(!extra2->free);

    var_extra_destroy(extra2);
    var_extra_destroy(extra);
mu_test_end()

/**
 * Tests cloning with value, clone operation, and destructor.
 */
mu_test_begin(test_var_extra_clone3)
    struct var_extra_t *extra
        = var_extra_create(strsave("abc"), my_str_clone, free);
    struct var_extra_t *extra2;

    mu_assert_eq_str("abc", extra->data);
    mu_assert(extra->clone == my_str_clone);
    mu_assert(extra->free == free);

    mu_assert_eq_int(0, my_clone_calls);
    extra2 = var_extra_clone(extra);
    mu_assert_eq_int(1, my_clone_calls);
    mu_assert_eq_str("abc", extra2->data);
    mu_assert(extra->clone == my_str_clone);
    mu_assert(extra->free == free);

    var_extra_destroy(extra2);
    var_extra_destroy(extra);
mu_test_end()

void
test_var_extra(void)
{
    mu_run_test(test_var_extra_ctor_dtor1);
    mu_run_test(test_var_extra_ctor_dtor2);
    mu_run_test(test_var_extra_ctor_dtor3);
    mu_run_test(test_var_extra_clone1);
    mu_run_test(test_var_extra_clone2);
    mu_run_test(test_var_extra_clone3);
}
