/*****************************************************************************
 *  @file expression/expr_referencer.h
 *  Referencer functions for expressions. These functions are responsible for
 *  resolving all unresolved variable expressions found within an expression.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_REFERENCER_H_
#define MKFLI4L_EXPRESSION_EXPR_REFERENCER_H_

#include <expression/expr.h>

/**
 * Resolves all unresolved variable expressions within passed expression.
 *
 * @param expr
 *  The expression where to look for unresolved variable expressions.
 * @param error
 *  Receives the name of the first unresolved variable which could not be
 *  resolved.
 * @return
 *  TRUE if all unresolved variable expressions could be resolved, FALSE
 *  otherwise. In the latter case, "error" is set to the name of the first
 *  unresolved variable.
 */
BOOL
expr_resolve_variable_expressions(struct expr_t *expr, char const **error);

#endif
