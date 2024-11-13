/*****************************************************************************
 *  @file expression/expr_typechecker.c
 *  Type checker for expressions.
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

#include <expression.h>
#include <expression/expr_internal.h>

/**
 * The underlying visitor.
 */
struct expr_typechecker_visitor_t
{
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Receives the first failing expression (if any).
     */
    struct expr_t *error;
};

/**
 * Handles a literal expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  Always TRUE.
 */
static BOOL
handle_literal(struct expr_visitor_t *v, struct  expr_literal_t *expr)
{
    return TRUE;
    UNUSED(v);
    UNUSED(expr);
}

/**
 * Handles a logical NOT expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_logical_not(struct expr_visitor_t *v, struct expr_logical_not_t *expr)
{
    struct expr_t *arg = expr_logical_not_get_arg(expr);
    if (!expr_accept(arg, v)) {
        return FALSE;
    }

    if (!expr_is_assignable_to(arg->type, EXPR_TYPE_BOOLEAN)) {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_logical_not_to_expr(expr);
        return FALSE;
    }

    expr_logical_not_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
    return TRUE;
}

/**
 * Handles a logical AND expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_logical_and(struct expr_visitor_t *v, struct expr_logical_and_t *expr)
{
    struct expr_t *left;
    struct expr_t *right;

    left = expr_logical_and_get_left_arg(expr);
    if (!expr_accept(left, v)) {
        return FALSE;
    }
    right = expr_logical_and_get_right_arg(expr);
    if (!expr_accept(right, v)) {
        return FALSE;
    }

    if (!expr_is_assignable_to(left->type, EXPR_TYPE_BOOLEAN) ||
            !expr_is_assignable_to(right->type, EXPR_TYPE_BOOLEAN)) {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_logical_and_to_expr(expr);
        return FALSE;
    }

    expr_logical_and_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
    return TRUE;
}

/**
 * Handles a logical OR expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_logical_or(struct expr_visitor_t *v, struct expr_logical_or_t *expr)
{
    struct expr_t *left;
    struct expr_t *right;

    left = expr_logical_or_get_left_arg(expr);
    if (!expr_accept(left, v)) {
        return FALSE;
    }
    right = expr_logical_or_get_right_arg(expr);
    if (!expr_accept(right, v)) {
        return FALSE;
    }

    if (!expr_is_assignable_to(left->type, EXPR_TYPE_BOOLEAN) ||
            !expr_is_assignable_to(right->type, EXPR_TYPE_BOOLEAN)) {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_logical_or_to_expr(expr);
        return FALSE;
    }

    expr_logical_or_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
    return TRUE;
}

/**
 * Handles an equality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_equal(struct expr_visitor_t *v, struct expr_equal_t *expr)
{
    struct expr_t *left;
    struct expr_t *right;

    left = expr_equal_get_left_arg(expr);
    if (!expr_accept(left, v)) {
        return FALSE;
    }
    right = expr_equal_get_right_arg(expr);
    if (!expr_accept(right, v)) {
        return FALSE;
    }

    if (expr_is_assignable_to(left->type, right->type) ||
            expr_is_assignable_to(right->type, left->type)) {
        expr_equal_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
        return TRUE;
    }
    else {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_equal_to_expr(expr);
        return FALSE;
    }
}

/**
 * Handles an inequality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_unequal(struct expr_visitor_t *v, struct expr_unequal_t *expr)
{
    struct expr_t *left;
    struct expr_t *right;

    left = expr_unequal_get_left_arg(expr);
    if (!expr_accept(left, v)) {
        return FALSE;
    }
    right = expr_unequal_get_right_arg(expr);
    if (!expr_accept(right, v)) {
        return FALSE;
    }

    if (expr_is_assignable_to(left->type, right->type) ||
            expr_is_assignable_to(right->type, left->type)) {
        expr_unequal_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
        return TRUE;
    }
    else {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_unequal_to_expr(expr);
        return FALSE;
    }
}

/**
 * Handles a match expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_match(struct expr_visitor_t *v, struct expr_match_t *expr)
{
    struct expr_t *left;
    struct expr_t *right;

    left = expr_match_get_left_arg(expr);
    if (!expr_accept(left, v)) {
        return FALSE;
    }
    right = expr_match_get_right_arg(expr);
    if (!expr_accept(right, v)) {
        return FALSE;
    }

    /* everything is assignable to EXPR_TYPE_STRING */
    my_assert(expr_is_assignable_to(right->type, EXPR_TYPE_STRING));

    expr_match_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
    return TRUE;
}

/**
 * Handles a TOBOOL expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_tobool(struct expr_visitor_t *v, struct expr_tobool_t *expr)
{
    struct expr_t *arg = expr_tobool_get_arg(expr);
    if (!expr_accept(arg, v)) {
        return FALSE;
    }

    if (!expr_is_assignable_to(arg->type, EXPR_TYPE_BOOLEAN)) {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_tobool_to_expr(expr);
        return FALSE;
    }

    expr_tobool_to_expr(expr)->type = EXPR_TYPE_BOOLEAN;
    return TRUE;
}

/**
 * Handles a variable expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if type-checking succeeded, else FALSE.
 */
static BOOL
handle_variable(struct expr_visitor_t *v, struct expr_variable_t *expr)
{
    if (expr_variable_to_expr(expr)->type == EXPR_TYPE_UNKNOWN) {
        struct expr_typechecker_visitor_t *const visitor
            = (struct expr_typechecker_visitor_t *) v;
        visitor->error = expr_variable_to_expr(expr);
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOL
expr_typecheck_expression(struct expr_t *expr, struct expr_t **error)
{
    struct expr_typechecker_visitor_t v;
    expr_visitor_init(
        &v.common,
        &handle_literal,
        &handle_logical_not,
        &handle_logical_and,
        &handle_logical_or,
        &handle_variable,
        &handle_equal,
        &handle_unequal,
        &handle_match,
        &handle_tobool
    );
    v.error = NULL;

    if (expr_accept(expr, &v.common)) {
        *error = NULL;
        return TRUE;
    }
    else {
        *error = v.error;
        return FALSE;
    }
}
