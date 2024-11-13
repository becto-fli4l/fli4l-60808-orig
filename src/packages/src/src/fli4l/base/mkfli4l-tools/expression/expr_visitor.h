/*****************************************************************************
 *  @file expression/expr_visitor.h
 *  Defines a visitor interface on expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_VISITOR_H_
#define MKFLI4L_EXPRESSION_EXPR_VISITOR_H_

#include <expression/expr.h>
#include <expression/expr_literal.h>
#include <expression/expr_logical_not.h>
#include <expression/expr_logical_and.h>
#include <expression/expr_logical_or.h>
#include <expression/expr_variable.h>
#include <expression/expr_equal.h>
#include <expression/expr_unequal.h>
#include <expression/expr_match.h>
#include <expression/expr_tobool.h>

/**
 * Represents an expression visitor.
 */
struct expr_visitor_t {
    /**
     * Handles a literal expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_literal)(struct expr_visitor_t *THIS, struct expr_literal_t *expr);

    /**
     * Handles a logical NOT expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_logical_not)(struct expr_visitor_t *THIS, struct expr_logical_not_t *expr);

    /**
     * Handles a logical AND expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_logical_and)(struct expr_visitor_t *THIS, struct expr_logical_and_t *expr);

    /**
     * Handles a logical OR expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_logical_or)(struct expr_visitor_t *THIS, struct expr_logical_or_t *expr);

    /**
     * Handles a variable expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_variable)(struct expr_visitor_t *THIS, struct expr_variable_t *expr);

    /**
     * Handles an equality comparison expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_equal)(struct expr_visitor_t *THIS, struct expr_equal_t *expr);

    /**
     * Handles an inequality comparison expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_unequal)(struct expr_visitor_t *THIS, struct expr_unequal_t *expr);

    /**
     * Handles a regular expression matching expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_match)(struct expr_visitor_t *THIS, struct expr_match_t *expr);

    /**
     * Handles a TOBOOL expression.
     * @param THIS
     *  This visitor.
     * @param expr
     *  The expression to handle.
     */
    BOOL (*handle_tobool)(struct expr_visitor_t *THIS, struct expr_tobool_t *expr);
};

/**
 * Initialises a visitor. A NULL handler method causes expr_accept() to return
 * FALSE.
 *
 * @param visitor
 *  The visitor to initialise.
 * @param handle_literal
 *  The handler for literal expressions.
 * @param handle_logical_not
 *  The handler for logical NOT expressions.
 * @param handle_logical_and
 *  The handler for logical AND expressions.
 * @param handle_logical_or
 *  The handler for logical OR expressions.
 * @param handle_variable
 *  The handler for variable expressions.
 * @param handle_equal
 *  The handler for equality expressions.
 * @param handle_unequal
 *  The handler for inequality expressions.
 * @param handle_match
 *  The handler for regular expression matching expressions.
 * @param handle_tobool
 *  The handler for TOBOOL expressions.
 */
void
expr_visitor_init(
    struct expr_visitor_t *visitor,
    BOOL (*handle_literal)(struct expr_visitor_t *THIS, struct expr_literal_t *expr),
    BOOL (*handle_logical_not)(struct expr_visitor_t *THIS, struct expr_logical_not_t *expr),
    BOOL (*handle_logical_and)(struct expr_visitor_t *THIS, struct expr_logical_and_t *expr),
    BOOL (*handle_logical_or)(struct expr_visitor_t *THIS, struct expr_logical_or_t *expr),
    BOOL (*handle_variable)(struct expr_visitor_t *THIS, struct expr_variable_t *expr),
    BOOL (*handle_equal)(struct expr_visitor_t *THIS, struct expr_equal_t *expr),
    BOOL (*handle_unequal)(struct expr_visitor_t *THIS, struct expr_unequal_t *expr),
    BOOL (*handle_match)(struct expr_visitor_t *THIS, struct expr_match_t *expr),
    BOOL (*handle_tobool)(struct expr_visitor_t *THIS, struct expr_tobool_t *expr)
);

#endif
