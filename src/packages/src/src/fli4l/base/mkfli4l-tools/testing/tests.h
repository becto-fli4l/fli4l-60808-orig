/*****************************************************************************
 *  @file tests.h
 *  Functions for testing mkfli4l.
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

#ifndef MKFLI4L_TESTING_TESTS_H_
#define MKFLI4L_TESTING_TESTS_H_

#include <testing/minunit.h>

/**
 * @name Test helpers.
 * @{
 */
/**
 * Creates a test location.
 *
 * @param type
 *  The file type to use.
 * @param line
 *  The line to use.
 * @return
 *  A suitable location_t object.
 */
struct location_t *
test_location_create(unsigned type, int line);
/**
 * @}
 */

/**
 * @name Module group 'container'.
 * @{
 */
/**
 * Tests comparison functions.
 */
void test_cmpfunc(void);
/**
 * Tests hash functions.
 */
void test_hashfunc(void);
/**
 * Tests hash maps.
 */
void test_hashmap(void);
/**
 * @}
 */

/**
 * @name Module group 'expression'.
 * @{
 */
/**
 * Tests expressions (common part).
 */
void test_expr(void);
/**
 * Tests literal expressions.
 */
void test_expr_literal(void);
/**
 * Tests literal error expressions.
 */
void test_expr_literal_error(void);
/**
 * Tests literal string expressions.
 */
void test_expr_literal_string(void);
/**
 * Tests literal integer expressions.
 */
void test_expr_literal_integer(void);
/**
 * Tests literal boolean expressions.
 */
void test_expr_literal_boolean(void);
/**
 * Tests logical NOT expressions.
 */
void test_expr_logical_not(void);
/**
 * Tests logical AND expressions.
 */
void test_expr_logical_and(void);
/**
 * Tests logical OR expressions.
 */
void test_expr_logical_or(void);
/**
 * Tests variable expressions.
 */
void test_expr_variable(void);
/**
 * Tests equality comparison expressions.
 */
void test_expr_equal(void);
/**
 * Tests inequality comparison expressions.
 */
void test_expr_unequal(void);
/**
 * Tests regular expression matching expressions.
 */
void test_expr_match(void);
/**
 * Tests TOBOOL expressions.
 */
void test_expr_tobool(void);
/**
 * Tests expression visitors.
 */
void test_expr_visitor(void);
/**
 * Tests the expression referencer.
 */
void test_expr_referencer(void);
/**
 * Tests the expression type-checker.
 */
void test_expr_typechecker(void);
/**
 * @}
 */

/**
 * @name Module group 'package'.
 * @{
 */
/**
 * Tests packages.
 */
void test_package_core(void);
/**
 * Tests package files.
 */
void test_package_file(void);
/**
 * Tests package locations.
 */
void test_package_location(void);
/**
 * @}
 */

/**
 * @name Module group 'variable'.
 * @{
 */
/**
 * Tests variable types.
 */
void test_vartype_core(void);
/**
 * Tests variable type extensions.
 */
void test_vartype_extension(void);
/**
 * Tests predefined variable types.
 */
void test_vartype_predef(void);
/**
 * @}
 */

/**
 * @name Module group 'var'.
 * @{
 */
/**
 * Tests variable property sets.
 */
void test_var_props(void);
/**
 * Tests a variable's extra data.
 */
void test_var_extra(void);
/**
 * Tests variables.
 */
void test_var_core(void);
/**
 * Tests variable conditions.
 */
void test_var_condition(void);
/**
 * Tests non-array variable slots.
 */
void test_var_slot_non_array(void);
/**
 * Tests array variable slots.
 */
void test_var_slot_array(void);
/**
 * Tests a variable's extra slot data.
 */
void test_var_slot_extra(void);
/**
 * Tests variable instantiations.
 */
void test_var_inst(void);
/**
 * Tests variable iterators.
 */
void test_var_iter(void);
/**
 * Tests variable scopes.
 */
void test_var_scope(void);
/**
 * Tests variable wrappers.
 */
void test_var_wrapper(void);
/**
 * Tests variable templates.
 */
void test_var_template(void);
/**
 * Tests variable parts.
 */
void test_var_part(void);
/**
 * @}
 */

/**
 * @name Module group 'grammar'.
 * @{
 */
/**
 * Tests scanner of variable type files.
 */
void test_vartype_scanner(void);
/**
 * Tests parser of variable type files.
 */
void test_vartype_parser(void);
/**
 * Tests scanner of variable definition files.
 */
void test_vardef_scanner(void);
/**
 * Tests parser of variable definition files.
 */
void test_vardef_parser(void);
/**
 * Tests scanner of variable assignment files.
 */
void test_varass_scanner(void);
/**
 * Tests parser of variable assignment files.
 */
void test_varass_parser(void);
/**
 * Tests scanner of expressions.
 */
void test_expr_scanner(void);
/**
 * Tests parser of expressions.
 */
void test_expr_parser(void);
/**
 * @}
 */

#endif
