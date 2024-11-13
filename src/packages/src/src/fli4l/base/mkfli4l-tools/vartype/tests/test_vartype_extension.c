/*****************************************************************************
 *  @file test_vartype_extension.c
 *  Functions for testing variable type extensions.
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
#include <var.h>
#include <package.h>
#include <expression.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "vartype.extension"

/**
 * Tests a single type extension which is enabled.
 */
mu_test_begin(test_vartype_extend1)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1)", "0 or 1 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert_eq_int(TRUE, vartype_extend(type, "(2|3)", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)), ", or 2 or 3 expected", "some other comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_int(TRUE, vartype_match(type, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type, "2"));
    mu_assert_eq_int(TRUE, vartype_match(type, "3"));
    mu_assert_eq_int(FALSE, vartype_match(type, "4"));

    regex = vartype_get_regex(type, TRUE);
    mu_assert_eq_str("((0|1))|((2|3))", regex);
    free(regex);
    mu_assert_eq_str("0 or 1 expected, or 2 or 3 expected", vartype_get_complete_error_message(type));
    mu_assert_eq_str("some comment\nsome other comment", vartype_get_complete_comment(type));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a single type extension which is disabled.
 */
mu_test_begin(test_vartype_extend2)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1)", "0 or 1 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert_eq_int(TRUE, vartype_extend(type, "(2|3)", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)), ", or 2 or 3 expected", "some other comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_int(TRUE, vartype_match(type, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type, "1"));
    mu_assert_eq_int(FALSE, vartype_match(type, "2"));
    mu_assert_eq_int(FALSE, vartype_match(type, "3"));

    regex = vartype_get_regex(type, TRUE);
    mu_assert_eq_str("(0|1)", regex);
    free(regex);
    mu_assert_eq_str("0 or 1 expected", vartype_get_complete_error_message(type));
    mu_assert_eq_str("some comment", vartype_get_complete_comment(type));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests two type extensions which are both enabled.
 */
mu_test_begin(test_vartype_extend3)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1)", "0 or 1 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert_eq_int(TRUE, vartype_extend(type, "2", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)), ", or 2 expected", "some other comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)));
    mu_assert_eq_int(TRUE, vartype_extend(type, "3", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)), ", or 3 expected", "another comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 44)));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_int(TRUE, vartype_match(type, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type, "2"));
    mu_assert_eq_int(TRUE, vartype_match(type, "3"));
    mu_assert_eq_int(FALSE, vartype_match(type, "4"));

    regex = vartype_get_regex(type, TRUE);
    mu_assert_eq_str("(((0|1))|(2))|(3)", regex);
    free(regex);
    mu_assert_eq_str("0 or 1 expected, or 2 expected, or 3 expected", vartype_get_complete_error_message(type));
    mu_assert_eq_str("some comment\nsome other comment\nanother comment", vartype_get_complete_comment(type));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a single type extension without an explicit condition.
 */
mu_test_begin(test_vartype_extend4)
    struct vartype_t *type;
    char *regex;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1)", "0 or 1 expected", "some comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert_eq_int(TRUE, vartype_extend(type, "(2|3)", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)), ", or 2 or 3 expected", "some other comment", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)));
    mu_assert(vartype_finalize_type_system());

    mu_assert_eq_int(TRUE, vartype_match(type, "0"));
    mu_assert_eq_int(TRUE, vartype_match(type, "1"));
    mu_assert_eq_int(TRUE, vartype_match(type, "2"));
    mu_assert_eq_int(TRUE, vartype_match(type, "3"));
    mu_assert_eq_int(FALSE, vartype_match(type, "4"));

    regex = vartype_get_regex(type, TRUE);
    mu_assert_eq_str("((0|1))|((2|3))", regex);
    free(regex);
    mu_assert_eq_str("0 or 1 expected, or 2 or 3 expected", vartype_get_complete_error_message(type));
    mu_assert_eq_str("some comment\nsome other comment", vartype_get_complete_comment(type));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a single type extension where the type has already been compiled.
 */
mu_test_begin(test_vartype_extend_compiled)
    struct vartype_t *type;
    struct location_t *loc;
    struct expr_t *expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1)", "0 or 1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert(vartype_finalize_type_system());

    expr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    loc = test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43);
    mu_assert_eq_int(FALSE, vartype_extend(type, "(2|3)", expr, ", or 2 or 3 expected", "", loc));
    expr_destroy(expr);
    location_destroy(loc);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a single type extension with a regular expression containing an
 * invalid reference.
 */
mu_test_begin(test_vartype_extend_invalid_expr)
    struct vartype_t *type;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType1", "(0|1)", "0 or 1 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    mu_assert_eq_int(TRUE, vartype_extend(type, "(RE:MyType2)", expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)), ", or MyType2 expected", "", test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)));
    mu_assert(!vartype_finalize_type_system());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which depends on a resolvable variable reference.
 */
mu_test_begin(test_vartype_extend_resolvable_var)
    struct vartype_t *type;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var1 = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    var2 = var_add(var_get_global_scope(), "B", vartype_get(VARTYPE_PREDEFINED_BOOLEAN), VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());
    var_write(var_get_name(var2), "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    inst = var_instance_create(var1, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), var_get_name(var2))),
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which depends on an unresolvable variable reference.
 */
mu_test_begin(test_vartype_extend_unresolvable_var)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "B")),
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which has a non-boolean condition.
 */
mu_test_begin(test_vartype_extend_nonbool_cond)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42)),
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which depends on a condition that is not
 * type-conformant.
 */
mu_test_begin(test_vartype_extend_incorrectly_typed_cond)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_logical_and_to_expr(expr_logical_and_create(var_get_global_scope(),
            expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "x")),
            expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "y"))
        )),
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which depends on a condition that cannot be evaluated.
 */
mu_test_begin(test_vartype_extend_eval_error)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_logical_and_to_expr(expr_logical_and_create(var_get_global_scope(),
            expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "error")),
            expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE))
        )),
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(!var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which depends on a NULL condition.
 */
mu_test_begin(test_vartype_extend_no_cond)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        NULL,
        ", or b expected",
        "; some comment",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a type extension which causes the value of its own enabling variable
 * to be properly typed.
 */
mu_test_begin(test_vartype_extend_own_dependency)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    var = var_add(var_get_global_scope(), "A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var, 0, NULL);
    var_instance_set_value(inst, "b", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type,
        "b",
        expr_equal_to_expr(expr_equal_create(var_get_global_scope(), expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var, 0, NULL))), expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "b")))),
        ", or b expected",
        "",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests two type extensions where each type extension causes the value of the
 * enabling variable of the other extension to be properly typed.
 */
mu_test_begin(test_vartype_extend_mutual_dependency)
    struct vartype_t *type1;
    struct vartype_t *type2;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *inst;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type1 = vartype_add("RA", "a", "a expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    type2 = vartype_add("RB", "b", "b expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43));
    var1 = var_add(var_get_global_scope(), "A", type1, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    var2 = var_add(var_get_global_scope(), "B", type2, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    inst = var_instance_create(var1, 0, NULL);
    var_instance_set_value(inst, "x", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);
    inst = var_instance_create(var2, 0, NULL);
    var_instance_set_value(inst, "y", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION);
    var_instance_destroy(inst);

    mu_assert(vartype_extend(
        type1,
        "x",
        expr_equal_to_expr(expr_equal_create(var_get_global_scope(), expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var2, 0, NULL))), expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "y")))),
        ", or x expected",
        "",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_extend(
        type2,
        "y",
        expr_equal_to_expr(expr_equal_create(var_get_global_scope(), expr_variable_to_expr(expr_variable_create(var_get_global_scope(), var_instance_create(var1, 0, NULL))), expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "x")))),
        ", or y expected",
        "",
        test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 43)
    ));

    mu_assert(vartype_finalize_type_system());
    mu_assert(var_check_values());

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_vartype_extension(void)
{
    set_log_level(LOG_EXP | LOG_REGEXP);
    mu_run_test(test_vartype_extend1);
    mu_run_test(test_vartype_extend2);
    mu_run_test(test_vartype_extend3);
    mu_run_test(test_vartype_extend4);
    mu_run_test(test_vartype_extend_compiled);
    mu_run_test(test_vartype_extend_invalid_expr);
    mu_run_test(test_vartype_extend_resolvable_var);
    mu_run_test(test_vartype_extend_unresolvable_var);
    mu_run_test(test_vartype_extend_nonbool_cond);
    mu_run_test(test_vartype_extend_incorrectly_typed_cond);
    mu_run_test(test_vartype_extend_eval_error);
    mu_run_test(test_vartype_extend_no_cond);
    mu_run_test(test_vartype_extend_own_dependency);
    mu_run_test(test_vartype_extend_mutual_dependency);
}
