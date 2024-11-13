/*****************************************************************************
 *  @file test_var_props.c
 *  Functions for testing variable property sets.
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
#include <expression/expr_literal.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "var.props"

/**
 * Tests the default property set.
 */
mu_test_begin(test_var_props_default)
    struct var_properties_t *props = var_properties_create();
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));

    mu_assert(!*props->comment);
    mu_assert(!props->optional);
    mu_assert(!props->weak);
    mu_assert(!props->def_value);
    mu_assert(!props->dis_value);
    mu_assert(!props->extra);
    mu_assert(expr_equal(expr_true, props->condition));

    expr_destroy(expr_true);
    var_properties_destroy(props);
mu_test_end()

/**
 * Tests cloning a property set.
 */
mu_test_begin(test_var_props_clone)
    struct var_extra_t *extra = var_extra_create("abc", NULL, NULL);
    struct var_properties_t *props1 = var_properties_create();
    struct var_properties_t *props2;

    props1->weak = TRUE;
    var_properties_set_comment(props1, "Blabla");
    var_properties_set_default_value(props1, expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")));
    var_properties_set_disabled_value(props1, "Welt");
    var_properties_set_condition(props1, expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE)));
    var_properties_set_extra(props1, extra);

    props2 = var_properties_clone(props1);
    mu_assert(props2);
    mu_assert_eq_str(props1->comment, props2->comment);
    mu_assert_eq_int(props1->optional, props2->optional);
    mu_assert_eq_int(props1->weak, props2->weak);
    mu_assert(props1->def_value != props2->def_value);
    mu_assert(expr_equal(props1->def_value, props2->def_value));
    mu_assert(props1->dis_value != props2->dis_value);
    mu_assert_eq_str(props1->dis_value, props2->dis_value);
    mu_assert(props1->condition != props2->condition);
    mu_assert(expr_equal(props1->condition, props2->condition));
    mu_assert(props1->extra != props2->extra);
    mu_assert_eq_str(props1->extra->data, props2->extra->data);

    var_properties_destroy(props2);
    var_properties_destroy(props1);
mu_test_end()

/**
 * Tests changing the comment in a property set.
 */
mu_test_begin(test_var_props_set_comment)
    struct var_properties_t *props = var_properties_create();

    mu_assert(!*props->comment);
    var_properties_set_comment(props, "Blabla");
    mu_assert_eq_str("Blabla", props->comment);
    var_properties_set_comment(props, "Blablubb");
    mu_assert_eq_str("Blablubb", props->comment);

    var_properties_destroy(props);
mu_test_end()

/**
 * Tests changing the default value in a property set.
 */
mu_test_begin(test_var_props_set_default_value)
    struct var_properties_t *props = var_properties_create();
    struct expr_literal_t *def_value;

    mu_assert(!props->def_value);
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")));
    def_value = expr_literal_create_string(var_get_global_scope(), "Hallo");
    mu_assert(expr_equal(expr_literal_to_expr(def_value), props->def_value));
    expr_destroy(expr_literal_to_expr(def_value));
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Welt")));
    def_value = expr_literal_create_string(var_get_global_scope(), "Welt");
    mu_assert(expr_equal(expr_literal_to_expr(def_value), props->def_value));
    expr_destroy(expr_literal_to_expr(def_value));

    var_properties_destroy(props);
mu_test_end()

/**
 * Tests changing the disabled value in a property set.
 */
mu_test_begin(test_var_props_set_disabled_value)
    struct var_properties_t *props = var_properties_create();

    mu_assert(!props->dis_value);
    var_properties_set_disabled_value(props, "Hallo");
    mu_assert_eq_str("Hallo", props->dis_value);
    var_properties_set_disabled_value(props, "Welt");
    mu_assert_eq_str("Welt", props->dis_value);

    var_properties_destroy(props);
mu_test_end()

/**
 * Tests changing the condition in a property set.
 */
mu_test_begin(test_var_props_set_condition)
    struct var_properties_t *props = var_properties_create();
    struct expr_t *expr_true
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    struct expr_t *expr_false
        = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    struct expr_t *cond;

    mu_assert(expr_equal(expr_true, props->condition));
    var_properties_set_condition(props, expr_false);
    cond = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    mu_assert(expr_equal(cond, props->condition));
    expr_destroy(cond);
    var_properties_set_condition(props, expr_true);
    cond = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_equal(cond, props->condition));
    expr_destroy(cond);

    var_properties_destroy(props);
mu_test_end()

/**
 * Tests changing extra data of a property set.
 */
mu_test_begin(test_var_props_set_extra)
    struct var_properties_t *props = var_properties_create();
    struct var_extra_t *extra;

    mu_assert(!props->extra);
    extra = var_extra_create("abc", NULL, NULL);
    var_properties_set_extra(props, extra);
    mu_assert(props->extra == extra);
    extra = var_extra_create("def", NULL, NULL);
    var_properties_set_extra(props, extra);
    mu_assert(props->extra == extra);

    var_properties_destroy(props);
mu_test_end()

/**
 * Tests checking default properties.
 */
mu_test_begin(test_var_props_check_valid1)
    struct var_properties_t *props = var_properties_create();
    mu_assert(var_properties_check(props, "V"));
    var_properties_destroy(props);
mu_test_end()

/**
 * Tests checking valid properties (1).
 */
mu_test_begin(test_var_props_check_valid2)
    struct var_properties_t *props = var_properties_create();
    props->optional = TRUE;
    mu_assert(var_properties_check(props, "V"));
    var_properties_destroy(props);
mu_test_end()

/**
 * Tests checking valid properties (2).
 */
mu_test_begin(test_var_props_check_valid3)
    struct var_properties_t *props = var_properties_create();
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")));
    mu_assert(var_properties_check(props, "V"));
    var_properties_destroy(props);
mu_test_end()

/**
 * Tests checking valid properties (3).
 */
mu_test_begin(test_var_props_check_valid4)
    struct var_properties_t *props = var_properties_create();
    props->optional = TRUE;
    var_properties_set_default_value(props, expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo")));
    mu_assert(var_properties_check(props, "V"));
    var_properties_destroy(props);
mu_test_end()

void
test_var_props(void)
{
    set_log_level(LOG_VAR);
    mu_run_test(test_var_props_default);
    mu_run_test(test_var_props_clone);
    mu_run_test(test_var_props_set_comment);
    mu_run_test(test_var_props_set_default_value);
    mu_run_test(test_var_props_set_disabled_value);
    mu_run_test(test_var_props_set_condition);
    mu_run_test(test_var_props_set_extra);
    mu_run_test(test_var_props_check_valid1);
    mu_run_test(test_var_props_check_valid2);
    mu_run_test(test_var_props_check_valid3);
    mu_run_test(test_var_props_check_valid4);
}
