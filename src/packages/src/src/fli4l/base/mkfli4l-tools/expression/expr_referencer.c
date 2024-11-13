/*****************************************************************************
 *  @file expression/expr_referencer.c
 *  Referencer functions for expressions.
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

/**
 * The underlying visitor.
 */
struct expr_referencer_visitor_t
{
    /**
     * Common visitor part.
     */
    struct expr_visitor_t common;
    /**
     * Receives the first failing variable (if any).
     */
    char const *error;
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
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_logical_not(struct expr_visitor_t *v, struct expr_logical_not_t *expr)
{
    return expr_accept(expr_logical_not_get_arg(expr), v);
}

/**
 * Handles a logical AND expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_logical_and(struct expr_visitor_t *v, struct expr_logical_and_t *expr)
{
    return expr_accept(expr_logical_and_get_left_arg(expr), v)
        ? expr_accept(expr_logical_and_get_right_arg(expr), v)
        : FALSE;
}

/**
 * Handles a logical OR expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_logical_or(struct expr_visitor_t *v, struct expr_logical_or_t *expr)
{
    return expr_accept(expr_logical_or_get_left_arg(expr), v)
        ? expr_accept(expr_logical_or_get_right_arg(expr), v)
        : FALSE;
}

/**
 * Handles an equality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_equal(struct expr_visitor_t *v, struct expr_equal_t *expr)
{
    return expr_accept(expr_equal_get_left_arg(expr), v)
        ? expr_accept(expr_equal_get_right_arg(expr), v)
        : FALSE;
}

/**
 * Handles an inequality comparison expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_unequal(struct expr_visitor_t *v, struct expr_unequal_t *expr)
{
    return expr_accept(expr_unequal_get_left_arg(expr), v)
        ? expr_accept(expr_unequal_get_right_arg(expr), v)
        : FALSE;
}

/**
 * Handles a match expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_match(struct expr_visitor_t *v, struct expr_match_t *expr)
{
    return expr_accept(expr_match_get_left_arg(expr), v)
        ? expr_accept(expr_match_get_right_arg(expr), v)
        : FALSE;
}

/**
 * Handles a TOBOOL expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_tobool(struct expr_visitor_t *v, struct expr_tobool_t *expr)
{
    return expr_accept(expr_tobool_get_arg(expr), v);
}

/**
 * Handles a variable expression.
 * @param v
 *  The visitor.
 * @param expr
 *  The expression.
 * @return
 *  TRUE if referencing succeeded, else FALSE.
 */
static BOOL
handle_variable(struct expr_visitor_t *v, struct expr_variable_t *expr)
{
    if (expr_variable_resolve(expr)) {
        return TRUE;
    }
    else {
        struct expr_referencer_visitor_t *const visitor
            = (struct expr_referencer_visitor_t *) v;
        visitor->error = expr_variable_get_unresolved_name(expr);
        return FALSE;
    }
}

BOOL
expr_resolve_variable_expressions(struct expr_t *expr, char const **error)
{
    struct expr_referencer_visitor_t v;
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
