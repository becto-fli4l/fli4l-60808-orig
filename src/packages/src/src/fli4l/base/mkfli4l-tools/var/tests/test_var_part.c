/*****************************************************************************
 *  @file test_var_part.c
 *  Functions for testing variable parts.
 *
 *  Copyright (c) 2016 The fli4l team
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
#include <expression.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.part"

/**
 * Tests converting a top-level variable part into a modern-style string.
 */
mu_test_begin(test_var_part_modern_toplevel)
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_top_level("ABC");
    name = var_part_to_string(part);
    mu_assert_eq_str("ABC", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a structure variable part into a modern-style string.
 */
mu_test_begin(test_var_part_modern_struct)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_struct(part_toplevel, "DEF");
    name = var_part_to_string(part);
    mu_assert_eq_str("ABC.DEF", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an array variable part into a modern-style string.
 */
mu_test_begin(test_var_part_modern_array)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_array(part_toplevel, 0);
    name = var_part_to_string(part);
    mu_assert_eq_str("ABC[]", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a mixed variable part into a modern-style string.
 */
mu_test_begin(test_var_part_modern_mixed)
    struct var_part_t *part_toplevel;
    struct var_part_t *part_array1;
    struct var_part_t *part_struct1;
    struct var_part_t *part_array2;
    struct var_part_t *part_struct2;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part_array1 = var_part_create_array(part_toplevel, 0);
    part_struct1 = var_part_create_struct(part_array1, "DEF");
    part_array2 = var_part_create_array(part_struct1, 0);
    part_struct2 = var_part_create_struct(part_array2, "GHI");
    name = var_part_to_string(part_struct2);
    mu_assert_eq_str("ABC[].DEF[].GHI", name);
    free(name);
    var_part_destroy(part_struct2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a top-level variable part into a classic-style string.
 */
mu_test_begin(test_var_part_classic_toplevel)
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_top_level("ABC");
    name = var_part_to_string_classic_style(part);
    mu_assert_eq_str("ABC", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a structure variable part into a classic-style string.
 */
mu_test_begin(test_var_part_classic_struct)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_struct(part_toplevel, "DEF");
    name = var_part_to_string_classic_style(part);
    mu_assert_eq_str("ABC_DEF", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an array variable part into a classic-style string.
 */
mu_test_begin(test_var_part_classic_array)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_array(part_toplevel, 0);
    name = var_part_to_string_classic_style(part);
    mu_assert_eq_str("ABC_%", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a mixed variable part into a classic-style string.
 */
mu_test_begin(test_var_part_classic_mixed)
    struct var_part_t *part_toplevel;
    struct var_part_t *part_array1;
    struct var_part_t *part_struct1;
    struct var_part_t *part_array2;
    struct var_part_t *part_struct2;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part_array1 = var_part_create_array(part_toplevel, 0);
    part_struct1 = var_part_create_struct(part_array1, "DEF");
    part_array2 = var_part_create_array(part_struct1, 0);
    part_struct2 = var_part_create_struct(part_array2, "GHI");
    name = var_part_to_string_classic_style(part_struct2);
    mu_assert_eq_str("ABC_%_DEF_%_GHI", name);
    free(name);
    var_part_destroy(part_struct2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable name into a variable part (1).
 */
mu_test_begin(test_var_part_from_classic1)
    struct var_part_t *part;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC");
    mu_assert(part);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, part->kind);
    mu_assert_eq_str("ABC", part->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable name into a variable part (2).
 */
mu_test_begin(test_var_part_from_classic2)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_DEF");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("DEF", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable name into a variable part (3).
 */
mu_test_begin(test_var_part_from_classic3)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_%");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(0, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable name into a variable part (4).
 */
mu_test_begin(test_var_part_from_classic4)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_%_DEF_%_GHI");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("GHI", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(0, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("DEF", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(0, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable name into a variable part (5).
 */
mu_test_begin(test_var_part_from_classic5)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_%_%_DEF_GHI");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("GHI", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("DEF", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(0, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(0, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an erroneous classic-style variable name into a variable
 * part (1).
 */
mu_test_begin(test_var_part_from_classic_error1)
    struct var_part_t *part;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("_DEF");
    mu_assert(!part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an erroneous classic-style variable name into a variable
 * part (2).
 */
mu_test_begin(test_var_part_from_classic_error2)
    struct var_part_t *part;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_");
    mu_assert(!part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an erroneous classic-style variable name into a variable
 * part (3).
 */
mu_test_begin(test_var_part_from_classic_error3)
    struct var_part_t *part;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC__DEF");
    mu_assert(!part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an array variable instance part into a modern-style string.
 */
mu_test_begin(test_var_inst_part_modern_array)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_array(part_toplevel, 2);
    name = var_part_to_string(part);
    mu_assert_eq_str("ABC[2]", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a mixed variable instance part into a modern-style string.
 */
mu_test_begin(test_var_inst_part_modern_mixed)
    struct var_part_t *part_toplevel;
    struct var_part_t *part_array1;
    struct var_part_t *part_struct1;
    struct var_part_t *part_array2;
    struct var_part_t *part_struct2;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part_array1 = var_part_create_array(part_toplevel, 3);
    part_struct1 = var_part_create_struct(part_array1, "DEF");
    part_array2 = var_part_create_array(part_struct1, 42);
    part_struct2 = var_part_create_struct(part_array2, "GHI");
    name = var_part_to_string(part_struct2);
    mu_assert_eq_str("ABC[3].DEF[42].GHI", name);
    free(name);
    var_part_destroy(part_struct2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting an array variable instance part into a classic-style string.
 */
mu_test_begin(test_var_inst_part_classic_array)
    struct var_part_t *part_toplevel;
    struct var_part_t *part;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part = var_part_create_array(part_toplevel, 2);
    name = var_part_to_string_classic_style(part);
    mu_assert_eq_str("ABC_2", name);
    free(name);
    var_part_destroy(part);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a mixed variable instance part into a classic-style string.
 */
mu_test_begin(test_var_inst_part_classic_mixed)
    struct var_part_t *part_toplevel;
    struct var_part_t *part_array1;
    struct var_part_t *part_struct1;
    struct var_part_t *part_array2;
    struct var_part_t *part_struct2;
    char *name;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part_toplevel = var_part_create_top_level("ABC");
    part_array1 = var_part_create_array(part_toplevel, 3);
    part_struct1 = var_part_create_struct(part_array1, "DEF");
    part_array2 = var_part_create_array(part_struct1, 42);
    part_struct2 = var_part_create_struct(part_array2, "GHI");
    name = var_part_to_string_classic_style(part_struct2);
    mu_assert_eq_str("ABC_3_DEF_42_GHI", name);
    free(name);
    var_part_destroy(part_struct2);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable instance name into a variable part
 * (1).
 */
mu_test_begin(test_var_inst_part_from_classic1)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_2");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(2, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable instance name into a variable part
 * (2).
 */
mu_test_begin(test_var_inst_part_from_classic2)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_3_DEF_42_GHI");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("GHI", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(42, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("DEF", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(3, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests converting a classic-style variable instance name into a variable part
 * (3).
 */
mu_test_begin(test_var_inst_part_from_classic3)
    struct var_part_t *part;
    struct var_part_t *p;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    part = var_part_create_from_string_classic_style("ABC_1_111_DEF_GHI");

    p = part;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("GHI", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_STRUCTURE, p->kind);
    mu_assert_eq_str("DEF", p->u.structure.name);

    p = p->u.structure.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(111, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_ARRAY, p->kind);
    mu_assert_eq_int(1, p->u.array.index);

    p = p->u.array.parent;
    mu_assert(p);
    mu_assert_eq_int(VAR_PART_TOP_LEVEL, p->kind);
    mu_assert_eq_str("ABC", p->u.top_level.name);

    var_part_destroy(part);
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_part(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_part_modern_toplevel);
    mu_run_test(test_var_part_modern_struct);
    mu_run_test(test_var_part_modern_array);
    mu_run_test(test_var_part_modern_mixed);
    mu_run_test(test_var_part_classic_toplevel);
    mu_run_test(test_var_part_classic_struct);
    mu_run_test(test_var_part_classic_array);
    mu_run_test(test_var_part_classic_mixed);
    mu_run_test(test_var_part_from_classic1);
    mu_run_test(test_var_part_from_classic2);
    mu_run_test(test_var_part_from_classic3);
    mu_run_test(test_var_part_from_classic4);
    mu_run_test(test_var_part_from_classic5);
    mu_run_test(test_var_part_from_classic_error1);
    mu_run_test(test_var_part_from_classic_error2);
    mu_run_test(test_var_part_from_classic_error3);
    mu_run_test(test_var_inst_part_modern_array);
    mu_run_test(test_var_inst_part_modern_mixed);
    mu_run_test(test_var_inst_part_classic_array);
    mu_run_test(test_var_inst_part_classic_mixed);
    mu_run_test(test_var_inst_part_from_classic1);
    mu_run_test(test_var_inst_part_from_classic2);
    mu_run_test(test_var_inst_part_from_classic3);
}
