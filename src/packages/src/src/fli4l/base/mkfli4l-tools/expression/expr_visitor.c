/*****************************************************************************
 *  @file expression/expr_visitor.c
 *  Visitor functions on expressions.
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

#include <expression/expr_visitor.h>

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
)
{
    visitor->handle_literal = handle_literal;
    visitor->handle_logical_not = handle_logical_not;
    visitor->handle_logical_and = handle_logical_and;
    visitor->handle_logical_or = handle_logical_or;
    visitor->handle_variable = handle_variable;
    visitor->handle_equal = handle_equal;
    visitor->handle_unequal = handle_unequal;
    visitor->handle_match = handle_match;
    visitor->handle_tobool = handle_tobool;
}
