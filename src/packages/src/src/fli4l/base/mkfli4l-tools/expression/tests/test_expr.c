/*****************************************************************************
 *  @file test_expr.c
 *  Functions for testing expressions.
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
#include <expression/expr.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr"

/**
 * Tests expr_type_to_string().
 */
mu_test_begin(test_expr_type_to_string)
    mu_assert_eq_str("error", expr_type_to_string(EXPR_TYPE_ERROR));
    mu_assert_eq_str("string", expr_type_to_string(EXPR_TYPE_STRING));
    mu_assert_eq_str("integer", expr_type_to_string(EXPR_TYPE_INTEGER));
    mu_assert_eq_str("boolean", expr_type_to_string(EXPR_TYPE_BOOLEAN));
mu_test_end()

void
test_expr(void)
{
    mu_run_test(test_expr_type_to_string);
}
