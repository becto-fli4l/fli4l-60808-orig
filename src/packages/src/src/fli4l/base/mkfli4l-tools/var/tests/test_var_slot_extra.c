/*****************************************************************************
 *  @file test_var_slot_extra.c
 *  Functions for testing a variable's extra slot data.
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

#define MU_TESTSUITE "var.slot.extra"

static int my_destr_calls = 0;

static void
my_destructor(void *data)
{
    ++my_destr_calls;
    UNUSED(data);
}

/**
 * Tests creating and destroying without value and destructor.
 */
mu_test_begin(test_var_slot_extra_ctor_dtor1)
    struct var_slot_extra_t *extra = var_slot_extra_create(NULL, NULL);

    mu_assert(!extra->data);
    mu_assert(!extra->free);

    var_slot_extra_destroy(extra);
mu_test_end()

/**
 * Tests creating and destroying with value but without destructor.
 */
mu_test_begin(test_var_slot_extra_ctor_dtor2)
    struct var_slot_extra_t *extra = var_slot_extra_create("abc", NULL);

    mu_assert_eq_str("abc", extra->data);
    mu_assert(!extra->free);

    var_slot_extra_destroy(extra);
mu_test_end()

/**
 * Tests creating and destroying with value and destructor.
 */
mu_test_begin(test_var_slot_extra_ctor_dtor3)
    struct var_slot_extra_t *extra = var_slot_extra_create("abc", my_destructor);

    mu_assert_eq_str("abc", extra->data);
    mu_assert(extra->free == my_destructor);

    mu_assert_eq_int(0, my_destr_calls);
    var_slot_extra_destroy(extra);
    mu_assert_eq_int(1, my_destr_calls);
mu_test_end()

void
test_var_slot_extra(void)
{
    mu_run_test(test_var_slot_extra_ctor_dtor1);
    mu_run_test(test_var_slot_extra_ctor_dtor2);
    mu_run_test(test_var_slot_extra_ctor_dtor3);
}
