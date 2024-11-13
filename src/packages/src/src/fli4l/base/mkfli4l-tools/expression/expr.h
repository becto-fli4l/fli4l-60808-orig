/*****************************************************************************
 *  @file expression/expr.h
 *  Basic definitions for dealing with expressions.
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

#ifndef MKFLI4L_EXPRESSION_EXPR_H_
#define MKFLI4L_EXPRESSION_EXPR_H_

#include <libmkfli4l/defs.h>
#include <var.h>

struct var_scope_t;

/**
 * @name Expression types.
 * An expression type describes the possible values for an expression's
 * resulting value.
 * @{
 */
/**
 * Represents an unknown type.
 */
#define EXPR_TYPE_UNKNOWN -1
/**
 * Represents the type "error".
 */
#define EXPR_TYPE_ERROR 0
/**
 * Represents the type "string".
 */
#define EXPR_TYPE_STRING 1
/**
 * Represents the type "integer".
 */
#define EXPR_TYPE_INTEGER 2
/**
 * Represents the type "boolean".
 */
#define EXPR_TYPE_BOOLEAN 3
/**
 * @}
 */

/**
 * @name Expression kinds.
 * An expression kind describes the different expressions that are possible.
 * @{
 */
/**
 * Represents a literal expression.
 */
#define EXPR_KIND_LITERAL 1
/**
 * Represents a logical NOT expression.
 */
#define EXPR_KIND_LOGICAL_NOT 2
/**
 * Represents a logical AND expression.
 */
#define EXPR_KIND_LOGICAL_AND 3
/**
 * Represents a logical OR expression.
 */
#define EXPR_KIND_LOGICAL_OR 4
/**
 * Represents a variable expression.
 */
#define EXPR_KIND_VARIABLE 5
/**
 * Represents an equality comparison expression.
 */
#define EXPR_KIND_EQUAL 6
/**
 * Represents an inequality comparison expression.
 */
#define EXPR_KIND_UNEQUAL 7
/**
 * Represents a regular expression matching expression.
 */
#define EXPR_KIND_MATCH 8
/**
 * Represents a TOBOOL expression.
 */
#define EXPR_KIND_TOBOOL 9
/**
 * @}
 */

struct expr_t;
struct expr_literal_t;
struct expr_eval_context_t;
struct expr_visitor_t;

/**
 * Returns the type of an expression. There are three possibilities:
 *
 * - The (static) type is known a priori. Then it is returned. This is valid
 *   for literal expressions and variable expressions which are created by
 *   reference.
 * - The (static) type is not known yet. Then EXPR_TYPE_UNKNOWN is returned.
 *   The real type will be set by the type checker.
 *
 * Note that evaluating an expression may return a literal that is of a more
 * special type, e.g. evaluating a string expression may return an error
 * literal. It is guaranteed that the type of an evaluated expression is
 * assignable to its static type.
 *
 * @param expr
 *  The expression.
 * @return
 *  The type of the expression.
 */
int
expr_get_type(struct expr_t const *expr);

/**
 * Maps an expression type to a string.
 *
 * @param type
 *  The expression type.
 * @return
 *  A string describing the expression type.
 */
char const *
expr_type_to_string(int type);

/**
 * Returns the kind of an expression.
 *
 * @param expr
 *  The expression.
 * @return
 *  The kind of the expression.
 */
int
expr_get_kind(struct expr_t const *expr);

/**
 * Maps an expression kind to a string.
 *
 * @param kind
 *  The expression kind.
 * @return
 *  A string describing the expression kind.
 */
char const *
expr_kind_to_string(int kind);

/**
 * Returns the lexical scope of an expression.
 *
 * @param expr
 *  The expression.
 * @return
 *  The scope of the expression.
 */
struct var_scope_t *
expr_get_scope(struct expr_t const *expr);

/**
 * Asks an expression to accept a visitor.
 *
 * @param expr
 *  The expression.
 * @param visitor
 *  The visitor.
 * @return
 *  TRUE if the corresponding handler method returned TRUE, FALSE otherwise.
 *  Note that FALSE is also returned if no handle method has been registered
 *  for the kind of expression in question.
 */
BOOL
expr_accept(struct expr_t *expr, struct expr_visitor_t *visitor);

/**
 * Evaluates an expression. Only a successfully type-checked expression may be
 * evaluated.
 *
 * @param expr
 *  The expression to evaluate.
 * @param context
 *  The evaluation context. May be NULL.
 * @return
 *  The evaluated expression. If the evaluation fails, a literal expression of
 *  type EXPR_TYPE_ERROR is returned. Use expr_destroy() to destroy it if you
 *  don't need it anymore.
 */
struct expr_literal_t *
expr_evaluate(struct expr_t const *expr, struct expr_eval_context_t *context);

/**
 * Transforms an expression into a string.
 *
 * @param expr
 *  The expression to transform.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
char *
expr_to_string(struct expr_t const *expr);

/**
 * Returns TRUE if a value of some source type can be assigned to a variable
 * of some target type. Currently, EXPR_TYPE_ERROR is assignable to every other
 * type, and every type is assignable to EXPR_TYPE_STRING. Additionally, each
 * type is assignable to itself.
 *
 * @param sourceType
 *  The type of some hypothetical value to be assigned.
 * @param targetType
 *  The type of some hypothetical variable the value is assigned to.
 * @return
 *  TRUE if the assignment is allowed, FALSE otherwise.
 */
BOOL
expr_is_assignable_to(int sourceType, int targetType);

/**
 * Casts a literal expression into another type. This succeeds only if
 * expr_is_assignable_to(expr_get_type(expr), targetType) would return TRUE.
 * The expression is evaluated if not already done so.
 *
 * Note that error expressions can be cast to any type but retain their type,
 * so don't expect
 * <code>expr_get_type(expr_cast(..., targetType)) == targetType</code> to be
 * TRUE.
 *
 * @param expr
 *  The expression to convert.
 * @param targetType
 *  The target type.
 * @return
 *  The expression cast to the new type or NULL if the cast failed.
 */
struct expr_literal_t *
expr_cast(struct expr_literal_t const *expr, int targetType);

/**
 * Returns the string value of a literal expression.
 *
 * @param expr
 *  The expression.
 * @param result
 *  Points to an object receiving the string value.
 * @param error
 *  Receives the error message if evaluation fails due to an error value. The
 *  caller is responsible for free()ing this string after use.
 * @return
 *  TRUE if the evaluation succeeds and the value is a string value, FALSE
 *  otherwise. In the latter case, "error" will point to an error message after
 *  returning to the caller.
 */
BOOL
expr_get_string_value(struct expr_literal_t const *expr,
                      char **result, char **error);

/**
 * Returns the integer value of a literal expression.
 *
 * @param expr
 *  The expression.
 * @param result
 *  Points to an object receiving the integer value.
 * @param error
 *  Receives the error message if evaluation fails due to an error value. The
 *  caller is responsible for free()ing this string after use.
 * @return
 *  TRUE if the evaluation succeeds and the value is an integer value, FALSE
 *  otherwise. In the latter case, "error" will point to an error message after
 *  returning to the caller.
 */
BOOL
expr_get_integer_value(struct expr_literal_t const *expr,
                       int *result, char **error);

/**
 * Returns the boolean value of a literal expression.
 *
 * @param expr
 *  The expression.
 * @param result
 *  Points to an object receiving the boolean value.
 * @param error
 *  Receives the error message if evaluation fails due to an error value. The
 *  caller is responsible for free()ing this string after use.
 * @return
 *  TRUE if the evaluation succeeds and the value is a boolean value, FALSE
 *  otherwise. In the latter case, "error" will point to an error message after
 *  returning to the caller.
 */
BOOL
expr_get_boolean_value(struct expr_literal_t const *expr,
                       BOOL *result, char **error);

/**
 * Clones an expression.
 *
 * @param expr
 *  The expression to clone.
 * @return
 *  The cloned expression. Use expr_destroy() to destroy it if you don't need
 *  it anymore.
 */
struct expr_t *
expr_clone(struct expr_t const *expr);

/**
 * Compares two expressions. Note that two structurally equivalent expressions
 * may still be different because one of them has been referenced or
 * type-checked, but not the other one.
 *
 * @param expr1
 *  The first expression.
 * @param expr2
 *  The second expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
BOOL
expr_equal(struct expr_t const *expr1, struct expr_t const *expr2);

/**
 * Frees the memory associated with an expression.
 *
 * @param expr
 *  The expression.
 */
void
expr_destroy(struct expr_t *expr);

/**
 * The evaluation context. It contains data that may be needed while evaluating
 * an expression.
 */
struct expr_eval_context_t {
    /**
     * The variable instance in which context the expression is evaluated.
     * It is used if the expression contains references to array variables
     * in order to infer the correct indices. May be NULL.
     */
    struct var_instance_t *var_inst;
};

/**
 * Creates an empty evaluation context.
 *
 * @return
 *  An empty evaluation context.
 */
struct expr_eval_context_t *
expr_eval_context_create(void);

/**
 * Destroys an evaluation context.
 *
 * @param THIS
 *  The evaluation context to destroy.
 */
void
expr_eval_context_destroy(struct expr_eval_context_t *THIS);

#endif
