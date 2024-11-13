/*****************************************************************************
 *  @file test_var_condition.c
 *  Functions for testing variable conditions.
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
#include <var.h>
#include <expression.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.condition"

/**
 * Tests adding a simple non-array variable with a condition.
 */
mu_test_begin(test_var_cond_simple1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct expr_t *cond;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_condition(props, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var);
    mu_assert_eq_str("V", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert_eq_int(0, var_get_level(var));

    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);

    cond = var_get_condition(var);
    mu_assert(expr_equal(props->condition, cond));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding a one-dimensional array with a condition.
 */
mu_test_begin(test_var_cond_array1)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_properties_t const *propsNew;
    struct var_t *var;
    struct var_t *var_n;
    struct expr_t *cond;
    struct expr_t *cond_exp;
    struct expr_literal_t *def_value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props = var_properties_create();
    var_properties_set_condition(props, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var = var_add(var_get_global_scope(), "V_%", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props);
    mu_assert(var);
    mu_assert_eq_str("V_%", var_get_name(var));
    mu_assert(var_get_type(var) == type);
    mu_assert_eq_int(1, var_get_level(var));

    propsNew = var_get_properties(var);
    mu_assert(!propsNew->def_value);

    cond = var_get_condition(var);
    mu_assert(expr_equal(props->condition, cond));

    var_n = var_get("V_N");
    mu_assert(var_n);
    propsNew = var_get_properties(var_n);
    def_value = expr_literal_create_integer(var_get_global_scope(), 0);
    mu_assert(expr_equal(expr_literal_to_expr(def_value), propsNew->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    cond = var_get_condition(var_n);
    cond_exp = var_get_condition(var);
    mu_assert(expr_equal(cond_exp, cond));

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different conditions (one/one) (1).
 */
mu_test_begin(test_var_cond_array2a)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_properties_set_condition(props1, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)));
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    var_properties_set_condition(props2, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    cond_n = var_get_condition(var_n);
    cond_exp = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different conditions (one/one) (2).
 */
mu_test_begin(test_var_cond_array2b)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_properties_set_condition(props1, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    var_properties_set_condition(props2, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    cond_n = var_get_condition(var_n);
    cond_exp = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different conditions (one/one) (3).
 */
mu_test_begin(test_var_cond_array2c)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *cond1;
    struct expr_t *cond2;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    cond1 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    var_properties_set_condition(props1, cond1);
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    cond2 = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE)),
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE))
    ));
    var_properties_set_condition(props2, cond2);
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    cond_n = var_get_condition(var_n);
    cond_exp = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(),
        expr_tobool_to_expr(expr_tobool_create(var_get_global_scope(), expr_clone(cond1))),
        expr_clone(cond2)
    ));
    mu_assert(expr_typecheck_expression(cond_exp, &error_expr));
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different conditions (none/one).
 */
mu_test_begin(test_var_cond_array3)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    var_properties_set_condition(props2, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    cond_n = var_get_condition(var_n);
    cond_exp = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two one-dimensional arrays depending on the same array bound
 * but with different conditions (one/none).
 */
mu_test_begin(test_var_cond_array4)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_n;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();

    props1 = var_properties_create();
    var_properties_set_condition(props1, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_add(var_get_global_scope(), "V_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    props2 = var_properties_create();
    var_add(var_get_global_scope(), "V_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props2);

    var_n = var_get("V_N");
    mu_assert(var_n);
    cond_n = var_get_condition(var_n);
    cond_exp = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests the condition of implicitly generated array bounds when array clients
 * depend on other arrays. In this case, these array references are replaced
 * by their enabling conditions.
 */
mu_test_begin(test_var_cond_array_ref)
    struct vartype_t *type_bool;
    struct vartype_t *type_numeric;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_t *var_opt;
    struct var_t *var1;
    struct var_t *var2;
    struct var_t *var_n;
    struct var_instance_t *inst;
    struct expr_t *cond_n;
    struct expr_literal_t *cond_result;
    struct expr_eval_context_t *context;
    BOOL result;
    char *error;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();
    type_bool = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    type_numeric = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);

    var_opt = var_add(var_get_global_scope(), "OPT_X", type_bool, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_opt);

    props1 = var_properties_create();
    inst = var_instance_create(var_opt, 0, NULL);
    var_properties_set_condition(props1, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst)));
    props1->optional = TRUE;
    var1 = var_add(var_get_global_scope(), "V_%", type_bool, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), props1);
    mu_assert(var1);

    props2 = var_properties_create();
    inst = var_instance_create(var1, 0, NULL);
    var_properties_set_condition(props2, expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst)));
    var2 = var_add(var_get_global_scope(), "V_%_X", type_numeric, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 25), props2);
    mu_assert(var2);

    inst = var_instance_create(var_opt, 0, NULL);
    var_n = var_get("V_N");
    mu_assert(var_n);

    mu_assert(var_write("OPT_X", "yes", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, TRUE));
    cond_n = var_get_condition(var_n);
    context = expr_eval_context_create();
    cond_result = expr_evaluate(cond_n, context);
    mu_assert(expr_get_boolean_value(cond_result, &result, &error));
    mu_assert(result);
    expr_destroy(expr_literal_to_expr(cond_result));
    expr_eval_context_destroy(context);

    mu_assert(var_write("OPT_X", "no", NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_CONFIGURATION, TRUE));
    cond_n = var_get_condition(var_n);
    context = expr_eval_context_create();
    cond_result = expr_evaluate(cond_n, context);
    mu_assert(expr_get_boolean_value(cond_result, &result, &error));
    mu_assert(!result);
    expr_destroy(expr_literal_to_expr(cond_result));
    expr_eval_context_destroy(context);

    var_instance_destroy(inst);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests adding two simple variables which depend on each other. Reading those
 * variables must fail as we require that those variables are disabled in such
 * a case of mutual dependency in order to stop the recursion.
 */
mu_test_begin(test_var_cond_mutual)
    struct vartype_t *type;
    struct var_properties_t *props1;
    struct var_properties_t *props2;
    struct var_scope_t *scope;
    struct var_t *var1;
    struct var_t *var2;
    struct var_instance_t *varinst1;
    struct var_instance_t *varinst2;
    struct expr_t *cond1;
    struct expr_t *cond2;
    struct var_value_t *value;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    scope = var_get_global_scope();

    props1 = var_properties_create();
    var1 = var_add(scope, "V1", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props1);
    mu_assert(var1);
    props2 = var_properties_create();
    var2 = var_add(scope, "V2", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), props2);
    mu_assert(var2);

    varinst1 = var_instance_create(var1, 0, NULL);
    mu_assert(varinst1);
    varinst2 = var_instance_create(var2, 0, NULL);
    mu_assert(varinst2);

    cond1 = expr_variable_to_expr(expr_variable_create(scope, varinst2));
    mu_assert(cond1);
    cond2 = expr_variable_to_expr(expr_variable_create(scope, varinst1));
    mu_assert(cond2);

    props1 = var_properties_clone(var_get_properties(var1));
    var_properties_set_condition(props1, cond1);
    mu_assert(var_set_properties(var1, props1));

    props2 = var_properties_clone(var_get_properties(var2));
    var_properties_set_condition(props2, cond2);
    mu_assert(var_set_properties(var2, props2));

    mu_assert(var_instance_set_and_check_value(varinst1, "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));
    mu_assert(var_instance_set_and_check_value(varinst2, "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION));

    mu_assert(!var_instance_is_enabled(varinst1));
    value = var_instance_get_value(varinst1);
    mu_assert(!value);

    mu_assert(!var_instance_is_enabled(varinst2));
    value = var_instance_get_value(varinst2);
    mu_assert(!value);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests whether conditions of array bounds are properly invalidated/updated if
 * deep clients have their conditions updated. The example is ("A --> B" means
 * "B is a client of A", "X{Y}" means "X is enabled by condition Y"):
 *
 * VAR_N
 *   --> VAR_%_N
 *         --> VAR_%_%_A{OPT_A}
 *         --> VAR_%_%_B{OPT_B}
 *   --> VAR_%_C{OPT_C}
 *
 * VAR_%_N's implicitly generated condition is:
 *   ([OPT_A]) || (OPT_B)
 *
 * VAR_N's implicitly generated condition is:
 *   ([([OPT_A]) || (OPT_B)]) || (OPT_C)
 *
 * If VAR_%_%_A's condition is changed to OPT_D, this change should be visible
 * in VAR_N's implicitly generated condition, which should be:
 *   ([([OPT_D]) || (OPT_B)]) || (OPT_C)
 *
 * This change should be propagated from grandchild client VAR_%_%_A over
 * child client VAR_%_N to VAR_N.
 */
mu_test_begin(test_var_cond_deep)
    struct vartype_t *type;
    struct var_properties_t *props;
    struct var_scope_t *scope;
    struct var_t *var_opta;
    struct var_t *var_optb;
    struct var_t *var_optc;
    struct var_t *var_optd;
    struct var_t *var_arr1;
    struct var_t *var_arr2;
    struct var_t *var_arr3;
    struct var_t *var_n1;
    struct var_t *var_n2;
    struct var_instance_t *varinst;
    struct var_instance_t *varinst_opta;
    struct var_instance_t *varinst_optb;
    struct var_instance_t *varinst_optc;
    struct var_instance_t *varinst_optd;
    struct expr_t *cond_n;
    struct expr_t *cond_exp;
    char const *error_var;
    struct expr_t *error_expr;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    type = vartype_get(VARTYPE_PREDEFINED_BOOLEAN);
    scope = var_get_global_scope();

    var_opta = var_add(scope, "OPT_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());
    mu_assert(var_opta);
    mu_assert(var_write("OPT_A", "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    var_optb = var_add(scope, "OPT_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 24), var_properties_create());
    mu_assert(var_optb);
    mu_assert(var_write("OPT_B", "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    var_optc = var_add(scope, "OPT_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 25), var_properties_create());
    mu_assert(var_optc);
    mu_assert(var_write("OPT_C", "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));
    var_optd = var_add(scope, "OPT_D", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 25), var_properties_create());
    mu_assert(var_optd);
    mu_assert(var_write("OPT_D", "yes", NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, TRUE));

    props = var_properties_create();
    varinst = var_instance_create(var_opta, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(scope, varinst)));
    var_arr1 = var_add(scope, "V_%_%_A", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 26), props);
    mu_assert(var_arr1);
    props = var_properties_create();
    varinst = var_instance_create(var_optb, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(scope, varinst)));
    var_arr2 = var_add(scope, "V_%_%_B", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 27), props);
    mu_assert(var_arr2);
    props = var_properties_create();
    varinst = var_instance_create(var_optc, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(scope, varinst)));
    var_arr3 = var_add(scope, "V_%_C", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 28), props);
    mu_assert(var_arr3);

    var_n2 = var_get("V_%_N");
    mu_assert(var_n2);

    varinst_opta = var_instance_create(var_opta, 0, NULL);
    varinst_optb = var_instance_create(var_optb, 0, NULL);
    cond_exp = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_tobool_to_expr(expr_tobool_create(scope,
            expr_variable_to_expr(expr_variable_create(scope, varinst_opta))
        )),
        expr_variable_to_expr(expr_variable_create(scope, varinst_optb))
    ));
    mu_assert(expr_resolve_variable_expressions(cond_exp, &error_var));
    mu_assert(expr_typecheck_expression(cond_exp, &error_expr));
    cond_n = var_get_condition(var_n2);
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_n1 = var_get("V_N");
    mu_assert(var_n1);

    varinst_opta = var_instance_create(var_opta, 0, NULL);
    varinst_optb = var_instance_create(var_optb, 0, NULL);
    varinst_optc = var_instance_create(var_optc, 0, NULL);
    cond_exp = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_tobool_to_expr(expr_tobool_create(scope,
            expr_logical_or_to_expr(expr_logical_or_create(scope,
                expr_tobool_to_expr(expr_tobool_create(scope,
                    expr_variable_to_expr(expr_variable_create(scope, varinst_opta))
                )),
                expr_variable_to_expr(expr_variable_create(scope, varinst_optb))
            ))
        )),
        expr_variable_to_expr(expr_variable_create(scope, varinst_optc))
    ));
    mu_assert(expr_resolve_variable_expressions(cond_exp, &error_var));
    mu_assert(expr_typecheck_expression(cond_exp, &error_expr));
    cond_n = var_get_condition(var_n1);
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    props = var_properties_clone(var_get_properties(var_arr1));
    varinst = var_instance_create(var_optd, 0, NULL);
    var_properties_set_condition(props, expr_variable_to_expr(expr_variable_create(scope, varinst)));
    mu_assert(var_set_properties(var_arr1, props));

    var_n2 = var_get("V_%_N");
    mu_assert(var_n2);

    varinst_optd = var_instance_create(var_optd, 0, NULL);
    varinst_optb = var_instance_create(var_optb, 0, NULL);
    cond_exp = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_tobool_to_expr(expr_tobool_create(scope,
            expr_variable_to_expr(expr_variable_create(scope, varinst_optd))
        )),
        expr_variable_to_expr(expr_variable_create(scope, varinst_optb))
    ));
    mu_assert(expr_resolve_variable_expressions(cond_exp, &error_var));
    mu_assert(expr_typecheck_expression(cond_exp, &error_expr));
    cond_n = var_get_condition(var_n2);
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_n1 = var_get("V_N");
    mu_assert(var_n1);

    varinst_optd = var_instance_create(var_optd, 0, NULL);
    varinst_optb = var_instance_create(var_optb, 0, NULL);
    varinst_optc = var_instance_create(var_optc, 0, NULL);
    cond_exp = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_tobool_to_expr(expr_tobool_create(scope,
            expr_logical_or_to_expr(expr_logical_or_create(scope,
                expr_tobool_to_expr(expr_tobool_create(scope,
                    expr_variable_to_expr(expr_variable_create(scope, varinst_optd))
                )),
                expr_variable_to_expr(expr_variable_create(scope, varinst_optb))
            ))
        )),
        expr_variable_to_expr(expr_variable_create(scope, varinst_optc))
    ));
    mu_assert(expr_resolve_variable_expressions(cond_exp, &error_var));
    mu_assert(expr_typecheck_expression(cond_exp, &error_expr));
    cond_n = var_get_condition(var_n1);
    mu_assert(expr_equal(cond_exp, cond_n));
    expr_destroy(cond_exp);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_var_condition(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_cond_simple1);
    mu_run_test(test_var_cond_array1);
    mu_run_test(test_var_cond_array2a);
    mu_run_test(test_var_cond_array2b);
    mu_run_test(test_var_cond_array2c);
    mu_run_test(test_var_cond_array3);
    mu_run_test(test_var_cond_array4);
    mu_run_test(test_var_cond_mutual);
    mu_run_test(test_var_cond_array_ref);
    mu_run_test(test_var_cond_deep);
}
