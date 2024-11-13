/*****************************************************************************
 *  @file expression/expr_variable.h
 *  Functions for processing variable expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_VARIABLE_H_
#define MKFLI4L_EXPRESSION_EXPR_VARIABLE_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>
#include <var.h>

struct expr_variable_t;

/**
 * Describes all possible variable expression states.
 */
enum expr_variable_state_kind_t {
    /**
     * The variable expression is unresolved yet. It references the variable to
     * evaluate by name. The referencer needs to be used in order to resolve
     * this variable expression.
     */
    EXPR_VARIABLE_STATE_UNRESOLVED,
    /**
     * The variable expression has been resolved successfully. It consists a
     * valid variable instance.
     */
    EXPR_VARIABLE_STATE_RESOLVED
};

/**
 * Creates a variable expression. A variable expression is an expression that
 * references the value of a variable. Its static type corresponds to the
 * type of the referenced variable. Hence, referencing and/or type-checking
 * such an expression is not necessary.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param instance
 *  The variable instance associated with the expression. Ownership is passed
 *  to the expression.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_variable_t *
expr_variable_create(
    struct var_scope_t *scope,
    struct var_instance_t *instance
);

/**
 * Creates a variable expression. A variable expression is an expression that
 * references the value of a variable. Note that a variable expression created
 * by this function has static type EXPR_TYPE_UNKNOWN until it is referenced
 * and type-checked successfully.
 *
 * @param scope
 *  The scope this expression lives in.
 * @param name
 *  The name of the variable instance associated with the expression. The
 *  referencer needs to resolve it later before this expression can be
 *  evaluated.
 * @return
 *  A pointer to a dynamically allocated expression. Use expr_free() to destroy
 *  it.
 */
struct expr_variable_t *
expr_variable_create_by_name(
    struct var_scope_t *scope,
    char const *name
);

/**
 * Casts a variable expression to a general expression.
 *
 * @param expr
 *  The expression to cast.
 * @return
 *  The cast expression.
 */
struct expr_t *
expr_variable_to_expr(struct expr_variable_t *expr);

/**
 * Returns the state of a variable expression.
 *
 * @param expr
 *  The variable expression.
 * @return
 *  The state of the expression.
 */
enum expr_variable_state_kind_t
expr_variable_get_state(struct expr_variable_t *expr);

/**
 * Returns the argument of a variable expression in state UNRESOLVED.
 *
 * @param expr
 *  The variable expression in state UNRESOLVED.
 * @return
 *  The argument of the expression.
 */
char const *
expr_variable_get_unresolved_name(struct expr_variable_t *expr);

/**
 * Returns the argument of a variable expression in state RESOLVED.
 *
 * @param expr
 *  The variable expression in state RESOLVED.
 * @return
 *  The argument of the expression.
 */
struct var_instance_t const *
expr_variable_get_resolved_instance(struct expr_variable_t *expr);

/**
 * Tries to resolve an unresolved variable expression. If the expression is
 * already resolved, nothing is done and TRUE is returned. Otherwise, the
 * variable is looked up by name and the variable changes its state
 * accordingly. Especially the variable expression's static type is set
 * according to the variable's type.
 *
 * @param expr
 *  The variable expression to resolve.
 * @return
 *  TRUE if resolution is successful, FALSE otherwise. In the first case, the
 *  expression's state changes to RESOLVED.
 */
BOOL
expr_variable_resolve(struct expr_variable_t *expr);

#endif
