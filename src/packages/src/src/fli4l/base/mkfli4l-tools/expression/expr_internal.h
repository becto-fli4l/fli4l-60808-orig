/*****************************************************************************
 *  @file expression/expr_internal.h
 *  Functions for processing expressions (private interface).
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

#ifndef MKFLI4L_EXPRESSION_EXPR_INTERNAL_H_
#define MKFLI4L_EXPRESSION_EXPR_INTERNAL_H_

#include <libmkfli4l/defs.h>
#include <expression/expr.h>

/**
 * Evaluates an expression.
 *
 * @param expr
 *  A pointer to an expression.
 * @param context
 *  A pointer to the evaluation context. May be NULL.
 * @return
 *  A pointer to a dynamically allocated literal expression describing the
 *  result. Use expr_free() to destroy it.
 */
typedef struct expr_literal_t *(*expr_evaluate_t)(struct expr_t const *expr,
                                                  struct expr_eval_context_t *context);

/**
 * Transforms an expression into a string.
 *
 * @param expr
 *  The expression to transform.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
typedef char *(*expr_to_string_t)(struct expr_t const *expr);

/**
 * Clones an expression.
 *
 * @param expr
 *  A pointer to an expression.
 * @return
 *  The cloned expression.
 */
typedef struct expr_t *(*expr_clone_t)(struct expr_t const *expr);

/**
 * Compares two expressions. Both expressions must be of the same kind and the
 * same type.
 *
 * @param expr1
 *  The first expression.
 * @param expr2
 *  The second expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
typedef BOOL (*expr_equal_t)(struct expr_t const *expr1, struct expr_t const *expr2);

/**
 * Destroys an expression.
 *
 * @param expr
 *  A pointer to an expression.
 */
typedef void (*expr_free_t)(struct expr_t *expr);

/**
 * Represents an abstract expression. Concrete expressions extend this
 * structure.
 */
struct expr_t {
    /**
     * The type of this expression, expressed by some EXPR_TYPE_* constant.
     */
    int type;
    /**
     * The kind of this expression, expressed by some EXPR_KIND_* constant.
     */
    int kind;
    /**
     * The scope of this expression.
     */
    struct var_scope_t *scope;
    /**
     * Evaluates this expression.
     */
    expr_evaluate_t evaluate;
    /**
     * Transforms this expression into a string.
     */
    expr_to_string_t toString;
    /**
     * Clones this expression.
     */
    expr_clone_t clone;
    /**
     * Compares two expressions.
     */
    expr_equal_t equal;
    /**
     * Destroys this expression.
     */
    expr_free_t free;
};

/**
 * Represents a literal expression.
 */
struct expr_literal_t {
    /**
     * Common part belonging to every expression. This member must be the first
     * one to allow casting between expr_t and expr_literal_t where necessary.
     */
    struct expr_t expr;
    /**
     * Union of the possible values of the literal expression.
     */
    union {
        /**
         * An error value. Set if expression type is EXPR_TYPE_ERROR.
         */
        char *error_value;
        /**
         * A string value. Set if expression type is EXPR_TYPE_STRING.
         */
        char *string_value;
        /**
         * An integer value. Set if expression type is EXPR_TYPE_INTEGER.
         */
        int int_value;
        /**
         * A boolean value. Set if expression type is EXPR_TYPE_BOOLEAN.
         */
        BOOL bool_value;
    } value;
};

/**
 * Initializes the common part of each expression. Each constructor of a
 * specific expression (e.g. expr_literal_create()) calls this function at its
 * beginning.
 *
 * @param expr
 *  The expression to initialize.
 * @param type
 *  The type of the expression.
 * @param kind
 *  The kind of the expression.
 * @param scope
 *  The scope of the expression.
 * @param evaluate
 *  A pointer to the evaluate() method for this expression.
 * @param toString
 *  A pointer to the toString() method for this expression.
 * @param clone
 *  A pointer to the clone() method for this expression.
 * @param equal
 *  A pointer to the equal() method for this expression.
 * @param free
 *  A pointer to the free() method for this expression.
 */
void
expr_common_init(
    struct expr_t *expr,
    int type,
    int kind,
    struct var_scope_t *scope,
    expr_evaluate_t evaluate,
    expr_to_string_t toString,
    expr_clone_t clone,
    expr_equal_t equal,
    expr_free_t free
);

/**
 * Destroys the expression. Each destructor of a specific expression (e.g.
 * expr_literal_free()) calls this function at its end.
 *
 * @param expr
 *  The expression to destroy.
 */
void
expr_common_free(struct expr_t *expr);

#endif
