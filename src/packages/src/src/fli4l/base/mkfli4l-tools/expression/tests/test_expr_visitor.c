/*****************************************************************************
 *  @file test_expr_visitor.c
 *  Functions for testing expression visitors.
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
#include <expression.h>
#include <package.h>
#include <vartype.h>
#include <var.h>
#include <libmkfli4l/str.h>
#include <stdlib.h>
#include <string.h>

#define MU_TESTSUITE "expression.expr_visitor"

/**
 * A visitor for literal expressions.
 */
struct my_literal_expr_visitor_t {
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Is set to TRUE by the visitor.
     */
    BOOL result;
};

/**
 * A visitor for expressions with one argument.
 */
struct my_one_arg_expr_visitor_t {
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Is set to TRUE by the visitor.
     */
    BOOL result;
    /**
     * The expression's argument.
     */
    struct expr_t *arg;
};

/**
 * A visitor for expressions with two arguments.
 */
struct my_two_args_expr_visitor_t {
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Is set to TRUE by the visitor.
     */
    BOOL result;
    /**
     * The expression's first argument.
     */
    struct expr_t *arg1;
    /**
     * The expression's second argument.
     */
    struct expr_t *arg2;
};

/**
 * A visitor for variable expressions.
 */
struct my_variable_expr_visitor_t {
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Is set to TRUE by the visitor.
     */
    BOOL result;
    /**
     * The expression's state.
     */
    enum expr_variable_state_kind_t state;
    /**
     * The expression's variable reference.
     */
    union {
        /**
         * Reference by name.
         */
        char const *name;
        /**
         * Reference by instance.
         */
        struct var_instance_t const *instance;
    } reference;
};

/**
 * Handles a literal expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_literal(struct expr_visitor_t *v, struct expr_literal_t *expr)
{
    ((struct my_literal_expr_visitor_t *) v)->result = TRUE;
    return TRUE;
    UNUSED(expr);
}

/**
 * Tests a visitor for a literal expression (1).
 */
mu_test_begin(test_expr_visitor_literal1)
    struct expr_t *expr;
    struct my_literal_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        &handle_literal,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    v.result = FALSE;
    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    expr_destroy(expr);

    v.result = FALSE;
    expr = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Error"));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    expr_destroy(expr);

    v.result = FALSE;
    expr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    expr_destroy(expr);

    v.result = FALSE;
    expr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a literal expression (2).
 */
mu_test_begin(test_expr_visitor_literal2)
    struct expr_t *expr;
    struct my_literal_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;

    expr = expr_literal_to_expr(expr_literal_create_error(var_get_global_scope(), "Error"));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Error"));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 42));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    expr_destroy(expr);

    expr = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a logical NOT expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_logical_not(struct expr_visitor_t *v, struct expr_logical_not_t *expr)
{
    struct my_one_arg_expr_visitor_t *visitor = (struct my_one_arg_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg = expr_logical_not_get_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for a logical NOT expression (1).
 */
mu_test_begin(test_expr_visitor_logical_not1)
    struct expr_t *arg;
    struct expr_t *expr;
    struct my_one_arg_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        &handle_logical_not,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg = NULL;

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg == arg);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a logical NOT expression (2).
 */
mu_test_begin(test_expr_visitor_logical_not2)
    struct expr_t *arg;
    struct expr_t *expr;
    struct my_one_arg_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg = NULL;

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_logical_not_to_expr(expr_logical_not_create(var_get_global_scope(), arg));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a logical AND expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_logical_and(struct expr_visitor_t *v, struct expr_logical_and_t *expr)
{
    struct my_two_args_expr_visitor_t *visitor = (struct my_two_args_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg1 = expr_logical_and_get_left_arg(expr);
    visitor->arg2 = expr_logical_and_get_right_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for a logical AND expression (1).
 */
mu_test_begin(test_expr_visitor_logical_and1)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        &handle_logical_and,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_and_to_expr(expr_logical_and_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg1 == arg1);
    mu_assert(v.arg2 == arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a logical AND expression (2).
 */
mu_test_begin(test_expr_visitor_logical_and2)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_and_to_expr(expr_logical_and_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg1);
    mu_assert(!v.arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a logical OR expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_logical_or(struct expr_visitor_t *v, struct expr_logical_or_t *expr)
{
    struct my_two_args_expr_visitor_t *visitor = (struct my_two_args_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg1 = expr_logical_or_get_left_arg(expr);
    visitor->arg2 = expr_logical_or_get_right_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for a logical OR expression (1).
 */
mu_test_begin(test_expr_visitor_logical_or1)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        &handle_logical_or,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg1 == arg1);
    mu_assert(v.arg2 == arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a logical OR expression (2).
 */
mu_test_begin(test_expr_visitor_logical_or2)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    arg2 = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
    expr = expr_logical_or_to_expr(expr_logical_or_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg1);
    mu_assert(!v.arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a variable expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_variable(struct expr_visitor_t *v, struct expr_variable_t *expr)
{
    struct my_variable_expr_visitor_t *visitor = (struct my_variable_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->state = expr_variable_get_state(expr);
    switch (visitor->state) {
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        visitor->reference.name = expr_variable_get_unresolved_name(expr);
        return TRUE;
    case EXPR_VARIABLE_STATE_RESOLVED :
        visitor->reference.instance = expr_variable_get_resolved_instance(expr);
        return TRUE;
    default :
        return FALSE;
    }
}

/**
 * Tests a visitor for an unresolved variable expression (1).
 */
mu_test_begin(test_expr_visitor_variable_unresolved1)
    struct expr_t *expr;
    struct my_variable_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_variable,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.state = (enum expr_variable_state_kind_t) -1;

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "ABC"));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_UNRESOLVED, v.state);
    mu_assert_eq_str("ABC", v.reference.name);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for an unresolved variable expression (2).
 */
mu_test_begin(test_expr_visitor_variable_unresolved2)
    struct expr_t *expr;
    struct my_variable_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.state = (enum expr_variable_state_kind_t) -1;

    expr = expr_variable_to_expr(expr_variable_create_by_name(var_get_global_scope(), "ABC"));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert_eq_int((enum expr_variable_state_kind_t) -1, v.state);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a resolved variable expression (1).
 */
mu_test_begin(test_expr_visitor_variable_resolved1)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct my_variable_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_variable,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.state = (enum expr_variable_state_kind_t) -1;

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert_eq_int(EXPR_VARIABLE_STATE_RESOLVED, v.state);
    mu_assert(v.reference.instance == inst);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a resolved variable expression (2).
 */
mu_test_begin(test_expr_visitor_variable_resolved2)
    struct vartype_t *type;
    struct var_t *var;
    struct var_instance_t *inst;
    struct expr_t *expr;
    struct my_variable_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    type = vartype_add("MyType", "(0|1|2|3)", "0 to 3 expected", "",  test_location_create(PACKAGE_FILE_TYPE_VARTYPE, 42));
    vartype_finalize_type_system();
    var = var_add(var_get_global_scope(), "V", type, VAR_PRIORITY_CONFIGURATION, test_location_create(PACKAGE_FILE_TYPE_VARDEF, 23), var_properties_create());

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.state = (enum expr_variable_state_kind_t) -1;

    inst = var_instance_create(var, 0, NULL);
    expr = expr_variable_to_expr(expr_variable_create(var_get_global_scope(), inst));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert_eq_int((enum expr_variable_state_kind_t) -1, v.state);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles an equality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_equal(struct expr_visitor_t *v, struct expr_equal_t *expr)
{
    struct my_two_args_expr_visitor_t *visitor = (struct my_two_args_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg1 = expr_equal_get_left_arg(expr);
    visitor->arg2 = expr_equal_get_right_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for an equality comparison expression (1).
 */
mu_test_begin(test_expr_visitor_equal1)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_equal,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg1 == arg1);
    mu_assert(v.arg2 == arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for an equality comparison expression (2).
 */
mu_test_begin(test_expr_visitor_equal2)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3));
    expr = expr_equal_to_expr(expr_equal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg1);
    mu_assert(!v.arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles an inequality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_unequal(struct expr_visitor_t *v, struct expr_unequal_t *expr)
{
    struct my_two_args_expr_visitor_t *visitor = (struct my_two_args_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg1 = expr_unequal_get_left_arg(expr);
    visitor->arg2 = expr_unequal_get_right_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for an inequality comparison expression (1).
 */
mu_test_begin(test_expr_visitor_unequal1)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_unequal,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3));
    expr = expr_unequal_to_expr(expr_unequal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg1 == arg1);
    mu_assert(v.arg2 == arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for an inequality comparison expression (2).
 */
mu_test_begin(test_expr_visitor_unequal2)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 2));
    arg2 = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 3));
    expr = expr_unequal_to_expr(expr_unequal_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg1);
    mu_assert(!v.arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a match expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_match(struct expr_visitor_t *v, struct expr_match_t *expr)
{
    struct my_two_args_expr_visitor_t *visitor = (struct my_two_args_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg1 = expr_match_get_left_arg(expr);
    visitor->arg2 = expr_match_get_right_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for a match expression (1).
 */
mu_test_begin(test_expr_visitor_match1)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_match,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo Welt"));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "^Hallo.*$"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg1, arg2));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg1 == arg1);
    mu_assert(v.arg2 == arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a match expression (2).
 */
mu_test_begin(test_expr_visitor_match2)
    struct expr_t *arg1;
    struct expr_t *arg2;
    struct expr_t *expr;
    struct my_two_args_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg1 = NULL;
    v.arg2 = NULL;

    arg1 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "Hallo Welt"));
    arg2 = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "^Hallo.*$"));
    expr = expr_match_to_expr(expr_match_create(var_get_global_scope(), arg1, arg2));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg1);
    mu_assert(!v.arg2);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Handles a TOBOOL expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 */
static BOOL
handle_tobool(struct expr_visitor_t *v, struct expr_tobool_t *expr)
{
    struct my_one_arg_expr_visitor_t *visitor = (struct my_one_arg_expr_visitor_t *) v;
    visitor->result = TRUE;
    visitor->arg = expr_tobool_get_arg(expr);
    return TRUE;
}

/**
 * Tests a visitor for a TOBOOL expression (1).
 */
mu_test_begin(test_expr_visitor_tobool1)
    struct expr_t *arg;
    struct expr_t *expr;
    struct my_one_arg_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &handle_tobool
    );
    v.result = FALSE;
    v.arg = NULL;

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_tobool_to_expr(expr_tobool_create(var_get_global_scope(), arg));
    mu_assert(expr_accept(expr, &v.common));
    mu_assert(v.result);
    mu_assert(v.arg == arg);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

/**
 * Tests a visitor for a TOBOOL expression (2).
 */
mu_test_begin(test_expr_visitor_tobool2)
    struct expr_t *arg;
    struct expr_t *expr;
    struct my_one_arg_expr_visitor_t v;

    package_init_module();
    vartype_init_module();
    var_init_module();
    vartype_finalize_type_system();

    expr_visitor_init(
        &v.common,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );
    v.result = FALSE;
    v.arg = NULL;

    arg = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    expr = expr_tobool_to_expr(expr_tobool_create(var_get_global_scope(), arg));
    mu_assert(!expr_accept(expr, &v.common));
    mu_assert(!v.result);
    mu_assert(!v.arg);
    expr_destroy(expr);

    var_fini_module();
    vartype_fini_module();
    package_fini_module();
mu_test_end()

void
test_expr_visitor(void)
{
    mu_run_test(test_expr_visitor_literal1);
    mu_run_test(test_expr_visitor_literal2);
    mu_run_test(test_expr_visitor_logical_not1);
    mu_run_test(test_expr_visitor_logical_not2);
    mu_run_test(test_expr_visitor_logical_and1);
    mu_run_test(test_expr_visitor_logical_and2);
    mu_run_test(test_expr_visitor_logical_or1);
    mu_run_test(test_expr_visitor_logical_or2);
    mu_run_test(test_expr_visitor_variable_unresolved1);
    mu_run_test(test_expr_visitor_variable_unresolved2);
    mu_run_test(test_expr_visitor_variable_resolved1);
    mu_run_test(test_expr_visitor_variable_resolved2);
    mu_run_test(test_expr_visitor_equal1);
    mu_run_test(test_expr_visitor_equal2);
    mu_run_test(test_expr_visitor_unequal1);
    mu_run_test(test_expr_visitor_unequal2);
    mu_run_test(test_expr_visitor_match1);
    mu_run_test(test_expr_visitor_match2);
    mu_run_test(test_expr_visitor_tobool1);
    mu_run_test(test_expr_visitor_tobool2);
}
