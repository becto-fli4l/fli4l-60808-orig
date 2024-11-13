/*****************************************************************************
 *  @file test_vartype_core.c
 *  Functions for testing variable types.
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
#include <expression/expr.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "vartype.core"

/**
 * Tests vartype_add() and vartype_get().
 */
mu_test_begin(test_vartype_add1)
    struct vartype_t *type1;
    struct vartype_t *type2;

    package_init_module();
    vartype_init_module();

    type1 = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type1);
    type2 = vartype_get("MyType");
    mu_assert(type1 == type2);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests redefinition via vartype_add().
 */
mu_test_begin(test_vartype_add2)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;
    struct location_t *loc;

    package_init_module();
    vartype_init_module();

    type1 = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type1);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42);
    type2 = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  loc);
    mu_assert(!type2);
    location_destroy(loc);
    loc = test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43);
    type3 = vartype_add("MyType", "(4|5|6|7)", "4 to 7 expected", "",  loc);
    mu_assert(!type3);
    location_destroy(loc);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_add() with an anonymous type.
 */
mu_test_begin(test_vartype_add_anonymous)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_add(NULL, "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type);

    vartype_fini_module();
    package_fini_module();
mu_test_end()


/**
 * Tests vartype_get_or_add().
 */
mu_test_begin(test_vartype_get_or_add)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_get_or_add("MyType");
    mu_assert(type);
    mu_assert_eq_str("", vartype_get_error_message(type));
    mu_assert(!vartype_get_regex(type, FALSE));

    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type);

    type = vartype_get_or_add("MyType");
    mu_assert(type);
    mu_assert_eq_str("0 to 3 expected", vartype_get_error_message(type));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_name().
 */
mu_test_begin(test_vartype_get_name)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type);
    mu_assert_eq_str("MyType", vartype_get_name(type));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_error_message().
 */
mu_test_begin(test_vartype_get_error_message)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type);
    mu_assert_eq_str("0 to 3 expected", vartype_get_error_message(type));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_complete_error_message().
 */
mu_test_begin(test_vartype_get_complete_error_message)
    struct vartype_t *type1;
    struct vartype_t *type2;

    package_init_module();
    vartype_init_module();

    type1 = vartype_add("MyType1", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_str("0 to 3 expected", vartype_get_complete_error_message(type1));
    mu_assert_eq_str("0 to 3 expected", vartype_get_complete_error_message(type2));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_comment().
 */
mu_test_begin(test_vartype_get_comment)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type);
    mu_assert_eq_str("some comment", vartype_get_comment(type));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_complete_comment().
 */
mu_test_begin(test_vartype_get_complete_comment)
    struct vartype_t *type1;
    struct vartype_t *type2;

    package_init_module();
    vartype_init_module();

    type1 = vartype_add("MyType1", "(0|1|2|3)", "0 to 3 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "(0|1|2|3)", "0 to 3 expected", "another comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_str("some comment", vartype_get_complete_comment(type1));
    mu_assert_eq_str("another comment", vartype_get_complete_comment(type2));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_regex().
 */
mu_test_begin(test_vartype_get_regex)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;
    char *regex;

    package_init_module();
    vartype_init_module();

    type1 = vartype_add("MyType1", "0|1|2|3", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(type1);
    type2 = vartype_add("MyType2", "4|5|6|7", "4 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    mu_assert(type2);
    type3 = vartype_add("MyType3", "(RE:MyType1)|(RE:MyType2)", "0 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(type3);

    regex = vartype_get_regex(type3, FALSE);
    mu_assert_eq_str("(RE:MyType1)|(RE:MyType2)", regex);
    free(regex);
    regex = vartype_get_regex(type3, TRUE);
    mu_assert_eq_str("(0|1|2|3)|(4|5|6|7)", regex);
    free(regex);

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_expression_type() on integer types.
 */
mu_test_begin(test_vartype_get_expression_type_integer)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
    mu_assert_eq_int(EXPR_TYPE_INTEGER, vartype_get_expression_type(type1));
    type2 = vartype_add("NUM_HEX", "0x[[:xdigit:]]+", "should be a hexadecimal number (a number starting with \"0x\")", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    mu_assert_eq_int(EXPR_TYPE_INTEGER, vartype_get_expression_type(type2));
    type3 = vartype_add("ENUMERIC", "()|(RE:" VARTYPE_PREDEFINED_UNSIGNED_INTEGER ")", "should be numeric (decimal) without leading zero(s) or empty", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert_eq_int(EXPR_TYPE_STRING, vartype_get_expression_type(type3));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_expression_type() on boolean types.
 */
mu_test_begin(test_vartype_get_expression_type_boolean)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    mu_assert_eq_int(EXPR_TYPE_BOOLEAN, vartype_get_expression_type(type1));
    type2 = vartype_add("YESNOEXT", "yes|no|maybe", "only yes, no, or maybe are allowed", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    mu_assert_eq_int(EXPR_TYPE_STRING, vartype_get_expression_type(type2));
    type3 = vartype_add("IMOND_YESNO", "yes", "only yes is allowed", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert_eq_int(EXPR_TYPE_STRING, vartype_get_expression_type(type3));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get_location().
 */
mu_test_begin(test_vartype_get_location)
    struct vartype_t *type;
    struct location_t *location;

    package_init_module();
    vartype_init_module();

    location = test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42);
    type = vartype_add("MyType1", "(0|1|2|3)", "0 to 3 expected", "",  location);
    mu_assert(vartype_finalize_type_system());

    mu_assert(location == vartype_get_location(type));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using groups, nested references, and alternatives.
 */
mu_test_begin(test_vartype_match1)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "4|5|6|7", "4 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "(RE:MyType1)|(RE:MyType2)", "0 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(TRUE, vartype_match(type1, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "2"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "3"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "00"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "11"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "22"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "33"));

    mu_assert(type2);
    mu_assert_eq_int(TRUE, vartype_match(type2, "4"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "5"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "6"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "7"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "44"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "55"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "66"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "77"));

    mu_assert(type3);
    mu_assert_eq_int(TRUE, vartype_match(type3, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "2"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "3"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "4"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "5"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "6"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "7"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "8"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using groups, sequences, and alternatives.
 */
mu_test_begin(test_vartype_match2)
    struct vartype_t *type1;
    struct vartype_t *type2;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "(00|01)", "00 or 01 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "0(0|1)", "00 or 01 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(FALSE, vartype_match(type1, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "00"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "01"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "001"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "010"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "011"));

    mu_assert(type2);
    mu_assert_eq_int(FALSE, vartype_match(type2, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "00"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "01"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "001"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "010"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "011"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using '*'.
 */
mu_test_begin(test_vartype_match3)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "0*1", "0*1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "01*", "01* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "0*1*", "0*1* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(TRUE, vartype_match(type1, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "001"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "0001"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "00"));

    mu_assert(type2);
    mu_assert_eq_int(TRUE, vartype_match(type2, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "011"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "0111"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "11"));

    mu_assert(type3);
    mu_assert_eq_int(TRUE, vartype_match(type3, ""));
    mu_assert_eq_int(TRUE, vartype_match(type3, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "00"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "11"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "0011"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "10"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using '+'.
 */
mu_test_begin(test_vartype_match4)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "0+1", "0+1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "01+", "01+ expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "0+1+", "0+1+ expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(FALSE, vartype_match(type1, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "001"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "0001"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "00"));

    mu_assert(type2);
    mu_assert_eq_int(FALSE, vartype_match(type2, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "011"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "0111"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "11"));

    mu_assert(type3);
    mu_assert_eq_int(FALSE, vartype_match(type3, ""));
    mu_assert_eq_int(FALSE, vartype_match(type3, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "00"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "11"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "01"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "0011"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "10"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using '?'.
 */
mu_test_begin(test_vartype_match5)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "0?1", "0?1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "01?", "01? expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "0?1?", "0?1? expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(TRUE, vartype_match(type1, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "01"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "001"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "0001"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "00"));

    mu_assert(type2);
    mu_assert_eq_int(TRUE, vartype_match(type2, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "01"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "011"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "0111"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "11"));

    mu_assert(type3);
    mu_assert_eq_int(TRUE, vartype_match(type3, ""));
    mu_assert_eq_int(TRUE, vartype_match(type3, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "00"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "11"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "01"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "0011"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "10"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using character classes '[...]'.
 */
mu_test_begin(test_vartype_match6)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "[0-1][2-3]", "[0-1][2-3] expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "[a-c]*", "[a-c]* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "[xyz]+", "[xyz]+ expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert_eq_int(TRUE, vartype_match(type1, "02"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "03"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "12"));
    mu_assert_eq_int(TRUE, vartype_match(type1, "13"));
    mu_assert_eq_int(FALSE, vartype_match(type1, ""));
    mu_assert_eq_int(FALSE, vartype_match(type1, "0"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "2"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "00"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "22"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "012"));
    mu_assert_eq_int(FALSE, vartype_match(type1, "123"));

    mu_assert(type2);
    mu_assert_eq_int(TRUE, vartype_match(type2, ""));
    mu_assert_eq_int(TRUE, vartype_match(type2, "a"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "b"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "c"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "aa"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "bb"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "cc"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "ab"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "bc"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "ca"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "ac"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "ba"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "cb"));
    mu_assert_eq_int(TRUE, vartype_match(type2, "abcbabccbbaa"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "d"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "A"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "ABC"));
    mu_assert_eq_int(FALSE, vartype_match(type2, "0"));

    mu_assert(type3);
    mu_assert_eq_int(TRUE, vartype_match(type3, "x"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "y"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "z"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "xxx"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "xyz"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "zyx"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "zzz"));
    mu_assert_eq_int(TRUE, vartype_match(type3, "yyyyy"));
    mu_assert_eq_int(FALSE, vartype_match(type3, ""));
    mu_assert_eq_int(FALSE, vartype_match(type3, "X"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "Y"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "Z"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "XYZ"));
    mu_assert_eq_int(FALSE, vartype_match(type3, "012"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() using escaping.
 */
mu_test_begin(test_vartype_match7)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();
    type = vartype_add("MyType", "0\\[abc\\]\\+\\(d\\)\\?1\\*", "0[abc]+d?1* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type);
    mu_assert_eq_int(TRUE, vartype_match(type, "0[abc]+(d)?1*"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0a+(d)?1*"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0b+(d)?1*"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0aaa(d)?1*"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0[abc]+1*"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0[abc]+(d)?"));
    mu_assert_eq_int(FALSE, vartype_match(type, "0[abc]+(d)?111"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match() against an anonymous type.
 */
mu_test_begin(test_vartype_match_anonymous)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();

    type = vartype_add(NULL, "0*", "0* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type);
    mu_assert(vartype_match(type, ""));
    mu_assert(vartype_match(type, "0"));
    mu_assert(vartype_match(type, "00"));
    mu_assert(vartype_match(type, "00000"));
    mu_assert(!vartype_match(type, "1"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match_ext() using groups, nested references, and alternatives.
 */
mu_test_begin(test_vartype_match_ext1)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct vartype_t *type3;
    regmatch_t *matches;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "4|5|6|7", "4 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    type3 = vartype_add("MyType3", "(RE:MyType1)|(RE:MyType2)", "0 to 7 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert(matches = vartype_match_ext(type1, "0"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type1, "1"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type1, "2"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type1, "3"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type1, "00"));
    mu_assert(!vartype_match_ext(type1, "11"));
    mu_assert(!vartype_match_ext(type1, "22"));
    mu_assert(!vartype_match_ext(type1, "33"));

    mu_assert(type2);
    mu_assert(matches = vartype_match_ext(type2, "4"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type2, "5"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type2, "6"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type2, "7"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type2, "44"));
    mu_assert(!vartype_match_ext(type2, "55"));
    mu_assert(!vartype_match_ext(type2, "66"));
    mu_assert(!vartype_match_ext(type2, "77"));

    mu_assert(type3);
    mu_assert(matches = vartype_match_ext(type3, "0"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(0, matches[3].rm_so);
    mu_assert_eq_int(1, matches[3].rm_eo);
    mu_assert_eq_int(-1, matches[4].rm_so);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "1"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(0, matches[3].rm_so);
    mu_assert_eq_int(1, matches[3].rm_eo);
    mu_assert_eq_int(-1, matches[4].rm_so);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "2"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(0, matches[3].rm_so);
    mu_assert_eq_int(1, matches[3].rm_eo);
    mu_assert_eq_int(-1, matches[4].rm_so);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "3"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(1, matches[2].rm_eo);
    mu_assert_eq_int(0, matches[3].rm_so);
    mu_assert_eq_int(1, matches[3].rm_eo);
    mu_assert_eq_int(-1, matches[4].rm_so);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "4"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    mu_assert_eq_int(-1, matches[3].rm_so);
    mu_assert_eq_int(0, matches[4].rm_so);
    mu_assert_eq_int(1, matches[4].rm_eo);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "5"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    mu_assert_eq_int(-1, matches[3].rm_so);
    mu_assert_eq_int(0, matches[4].rm_so);
    mu_assert_eq_int(1, matches[4].rm_eo);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "6"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    mu_assert_eq_int(-1, matches[3].rm_so);
    mu_assert_eq_int(0, matches[4].rm_so);
    mu_assert_eq_int(1, matches[4].rm_eo);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type3, "7"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    mu_assert_eq_int(-1, matches[3].rm_so);
    mu_assert_eq_int(0, matches[4].rm_so);
    mu_assert_eq_int(1, matches[4].rm_eo);
    mu_assert_eq_int(-1, matches[5].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type3, "8"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match_ext() using groups, sequences, and alternatives.
 */
mu_test_begin(test_vartype_match_ext2)
    struct vartype_t *type1;
    struct vartype_t *type2;
    regmatch_t *matches;

    package_init_module();
    vartype_init_module();
    type1 = vartype_add("MyType1", "(00|01)", "00 or 01 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("MyType2", "0(0|1)", "00 or 01 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type1);
    mu_assert(!vartype_match_ext(type1, "0"));
    mu_assert(!vartype_match_ext(type1, "1"));
    mu_assert(matches = vartype_match_ext(type1, "00"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(2, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(2, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(2, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type1, "01"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(2, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(2, matches[1].rm_eo);
    mu_assert_eq_int(0, matches[2].rm_so);
    mu_assert_eq_int(2, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type1, "001"));
    mu_assert(!vartype_match_ext(type1, "010"));
    mu_assert(!vartype_match_ext(type1, "011"));

    mu_assert(type2);
    mu_assert(!vartype_match_ext(type2, "0"));
    mu_assert(!vartype_match_ext(type2, "1"));
    mu_assert(matches = vartype_match_ext(type2, "00"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(2, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(2, matches[1].rm_eo);
    mu_assert_eq_int(1, matches[2].rm_so);
    mu_assert_eq_int(2, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type2, "01"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(2, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(2, matches[1].rm_eo);
    mu_assert_eq_int(1, matches[2].rm_so);
    mu_assert_eq_int(2, matches[2].rm_eo);
    mu_assert_eq_int(-1, matches[3].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type2, "001"));
    mu_assert(!vartype_match_ext(type2, "010"));
    mu_assert(!vartype_match_ext(type2, "011"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_match_ext() against an anonymous type.
 */
mu_test_begin(test_vartype_match_ext_anonymous)
    struct vartype_t *type;
    regmatch_t *matches;

    package_init_module();
    vartype_init_module();

    type = vartype_add(NULL, "0*", "0* expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    mu_assert(type);
    mu_assert(matches = vartype_match_ext(type, ""));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(0, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(0, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type, "0"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(1, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(1, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type, "00"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(2, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(2, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(matches = vartype_match_ext(type, "00000"));
    mu_assert_eq_int(0, matches[0].rm_so);
    mu_assert_eq_int(5, matches[0].rm_eo);
    mu_assert_eq_int(0, matches[1].rm_so);
    mu_assert_eq_int(5, matches[1].rm_eo);
    mu_assert_eq_int(-1, matches[2].rm_so);
    free(matches);
    mu_assert(!vartype_match_ext(type, "1"));

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests cycle of length 1 (direct recursion).
 */
mu_test_begin(test_vartype_cycles1)
    package_init_module();
    vartype_init_module();
    vartype_add("MyType", "(RE:MyType)", "Type expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests cycle of length 3 (indirect recursion).
 */
mu_test_begin(test_vartype_cycles2)
    package_init_module();
    vartype_init_module();
    vartype_add("MyType1", "(RE:MyType2)", "Type2 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_add("MyType2", "(RE:MyType3)", "Type3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    vartype_add("MyType3", "(RE:MyType1)", "Type1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests vartype_get() on missing type.
 */
mu_test_begin(test_vartype_missing1)
    package_init_module();
    vartype_init_module();
    mu_assert(vartype_finalize_type_system());
    mu_assert(!vartype_get("MyType1"));
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests referencing missing types by named types.
 */
mu_test_begin(test_vartype_missing2a)
    package_init_module();
    vartype_init_module();
    vartype_add("MyType1", "(RE:MyType2)", "Type2 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests referencing missing types by unnamed types.
 */
mu_test_begin(test_vartype_missing2b)
    package_init_module();
    vartype_init_module();
    vartype_add(NULL, "(RE:MyType2)", "Type2 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests syntactically broken references in named types.
 */
mu_test_begin(test_vartype_missing3a)
    package_init_module();
    vartype_init_module();
    vartype_add("MyType1", "(RE:MyType2", "Type2 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests syntactically broken references in unnamed types.
 */
mu_test_begin(test_vartype_missing3b)
    package_init_module();
    vartype_init_module();
    vartype_add(NULL, "(RE:MyType2", "Type2 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(!vartype_finalize_type_system());

    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_vartype_core(void)
{
    set_log_level(LOG_EXP | LOG_REGEXP);
    mu_run_test(test_vartype_add1);
    mu_run_test(test_vartype_add2);
    mu_run_test(test_vartype_add_anonymous);
    mu_run_test(test_vartype_get_or_add);
    mu_run_test(test_vartype_get_name);
    mu_run_test(test_vartype_get_error_message);
    mu_run_test(test_vartype_get_complete_error_message);
    mu_run_test(test_vartype_get_comment);
    mu_run_test(test_vartype_get_complete_comment);
    mu_run_test(test_vartype_get_regex);
    mu_run_test(test_vartype_get_expression_type_integer);
    mu_run_test(test_vartype_get_expression_type_boolean);
    mu_run_test(test_vartype_get_location);
    mu_run_test(test_vartype_match1);
    mu_run_test(test_vartype_match2);
    mu_run_test(test_vartype_match3);
    mu_run_test(test_vartype_match4);
    mu_run_test(test_vartype_match5);
    mu_run_test(test_vartype_match6);
    mu_run_test(test_vartype_match7);
    mu_run_test(test_vartype_match_anonymous);
    mu_run_test(test_vartype_match_ext1);
    mu_run_test(test_vartype_match_ext2);
    mu_run_test(test_vartype_match_ext_anonymous);
    mu_run_test(test_vartype_cycles1);
    mu_run_test(test_vartype_cycles2);
    mu_run_test(test_vartype_missing1);
    mu_run_test(test_vartype_missing2a);
    mu_run_test(test_vartype_missing2b);
    mu_run_test(test_vartype_missing3a);
    mu_run_test(test_vartype_missing3b);
}
