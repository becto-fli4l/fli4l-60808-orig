/*****************************************************************************
 *  @file expression/expr_typechecker.h
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

#ifndef MKFLI4L_EXPRESSION_EXPR_TYPECHECKER_H_
#define MKFLI4L_EXPRESSION_EXPR_TYPECHECKER_H_

#include <expression/expr.h>

/**
 * Checks whether the expression meets all typing requirements. Note that this
 * expression should have already been visited by the referencer in order to
 * know the types of variables used in the expression. Otherwise type-checking
 * will fail if unresolved variables are contained therein.
 *
 * @param expr
 *  The expression to check.
 * @param error
 *  Receives the first expression which does not fulfil the typing rules. The
 *  parameter points directly to the original expression, so the expression
 *  pointed to is valid only as long as "expr" is.
 * @return
 *  TRUE if the expression is type-conformant, FALSE otherwise. In the latter
 *  case, "error" is set to the first non-conformant (sub-)expression.
 */
BOOL
expr_typecheck_expression(struct expr_t *expr, struct expr_t **error);

#endif
