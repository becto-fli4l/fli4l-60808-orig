/*****************************************************************************
 *  @file expression/expr.c
 *  Functions for processing expressions.
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

#include <expression.h>
#include <expression/expr_internal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>

#define MODULE "expression::expr"

int
expr_get_type(struct expr_t const *expr)
{
    return expr->type;
}

char const *
expr_type_to_string(int type)
{
    switch (type) {
    case EXPR_TYPE_ERROR :
        return "error";
    case EXPR_TYPE_STRING :
        return "string";
    case EXPR_TYPE_INTEGER :
        return "integer";
    case EXPR_TYPE_BOOLEAN :
        return "boolean";
    default :
        my_assert(FALSE);
        return NULL;
    }
}

int
expr_get_kind(struct expr_t const *expr)
{
    return expr->kind;
}

char const *
expr_kind_to_string(int kind)
{
    switch (kind) {
    case EXPR_KIND_LITERAL :
        return "literal";
    case EXPR_KIND_LOGICAL_NOT :
        return "logical not";
    case EXPR_KIND_LOGICAL_AND :
        return "logical and";
    case EXPR_KIND_LOGICAL_OR :
        return "logical or";
    case EXPR_KIND_VARIABLE :
        return "variable";
    case EXPR_KIND_EQUAL :
        return "equal";
    case EXPR_KIND_UNEQUAL :
        return "unequal";
    case EXPR_KIND_MATCH :
        return "match";
    case EXPR_KIND_TOBOOL :
        return "tobool";
    default :
        my_assert(FALSE);
        return NULL;
    }
}

struct var_scope_t *
expr_get_scope(struct expr_t const *expr)
{
    return expr->scope;
}

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
)
{
    expr->type = type;
    expr->kind = kind;
    expr->scope = scope;
    expr->evaluate = evaluate;
    expr->toString = toString;
    expr->clone = clone;
    expr->equal = equal;
    expr->free = free;
}

void
expr_common_free(struct expr_t *expr)
{
    free(expr);
}

BOOL
expr_accept(struct expr_t *expr, struct expr_visitor_t *visitor)
{
    switch (expr->kind) {
    case EXPR_KIND_LITERAL :
        return visitor->handle_literal ?
            (*visitor->handle_literal)(visitor, (struct expr_literal_t *) expr) : FALSE;
    case EXPR_KIND_LOGICAL_NOT :
        return visitor->handle_logical_not ?
            (*visitor->handle_logical_not)(visitor, (struct expr_logical_not_t *) expr) : FALSE;
    case EXPR_KIND_LOGICAL_AND :
        return visitor->handle_logical_and ?
            (*visitor->handle_logical_and)(visitor, (struct expr_logical_and_t *) expr) : FALSE;
    case EXPR_KIND_LOGICAL_OR :
        return visitor->handle_logical_or ?
            (*visitor->handle_logical_or)(visitor, (struct expr_logical_or_t *) expr) : FALSE;
    case EXPR_KIND_VARIABLE :
        return visitor->handle_variable ?
            (*visitor->handle_variable)(visitor, (struct expr_variable_t *) expr) : FALSE;
    case EXPR_KIND_EQUAL :
        return visitor->handle_equal ?
            (*visitor->handle_equal)(visitor, (struct expr_equal_t *) expr) : FALSE;
    case EXPR_KIND_UNEQUAL :
        return visitor->handle_unequal ?
            (*visitor->handle_unequal)(visitor, (struct expr_unequal_t *) expr) : FALSE;
    case EXPR_KIND_MATCH :
        return visitor->handle_match ?
            (*visitor->handle_match)(visitor, (struct expr_match_t *) expr) : FALSE;
    case EXPR_KIND_TOBOOL :
        return visitor->handle_tobool ?
            (*visitor->handle_tobool)(visitor, (struct expr_tobool_t *) expr) : FALSE;
    default :
        my_assert(FALSE);
        return FALSE;
    }
}

struct expr_literal_t *
expr_evaluate(struct expr_t const *expr, struct expr_eval_context_t *context)
{
    struct expr_literal_t *result;
    my_assert(expr->type != EXPR_TYPE_UNKNOWN);
    result = (*expr->evaluate)(expr, context);
    my_assert(result != NULL);
    my_assert(expr_is_assignable_to(result->expr.type, expr->type));
    return result;
}

char *
expr_to_string(struct expr_t const *expr)
{
    return (*expr->toString)(expr);
}

BOOL
expr_is_assignable_to(int sourceType, int targetType)
{
    return sourceType == targetType ||
        sourceType == EXPR_TYPE_ERROR ||
        targetType == EXPR_TYPE_STRING;
}

struct expr_literal_t *
expr_cast(struct expr_literal_t const *expr, int targetType)
{
    if (expr_is_assignable_to(expr->expr.type, targetType)) {
        struct expr_literal_t *result = NULL;

        if (expr->expr.type == targetType || expr->expr.type == EXPR_TYPE_ERROR) {
            result = (struct expr_literal_t *) expr_clone(&expr->expr);
        }
        else {
            char *value;
            my_assert(targetType == EXPR_TYPE_STRING);
            value = expr_to_string(&expr->expr);
            result = expr_literal_create_string(expr->expr.scope, value);
            free(value);
        }

        return result;
    }
    else {
        return NULL;
    }
}

/**
 * Returns the value of a literal expression. If the type of the literal
 * expression is not identical to the target type, a type cast is performed if
 * possible (i.e. if expr_is_assignable_to(type_of_evaluated_expression,
 * targetType) returns TRUE), otherwise the function fails.
 *
 * @param expr
 *  The expression.
 * @param targetType
 *  The target type.
 * @param result
 *  Points to an object receiving the value.
 * @param error
 *  Receives the error message if evaluation fails due to an error value. The
 *  caller is responsible for free()ing this string after use.
 * @return
 *  TRUE if the evaluation succeeds and the type of the value is assignable to
 *  the target type, FALSE otherwise. In the latter case, "error" will point to
 *  an error message after returning to the caller.
 */
static BOOL
expr_get_value(struct expr_literal_t const *expr, int targetType,
               void *result, char **error)
{
    struct expr_literal_t *e = expr_cast(expr, targetType);
    if (e && e->expr.type == targetType) {
        expr_literal_get_value(e, result);
        expr_destroy(&e->expr);
        return TRUE;
    }
    else if (e) {
        expr_literal_get_value(e, error);
        expr_destroy(&e->expr);
        return FALSE;
    }
    else {
        *error = safe_sprintf(
            "invalid type cast from %s to %s",
            expr_type_to_string(expr->expr.type),
            expr_type_to_string(targetType)
        );
        return FALSE;
    }
}

BOOL
expr_get_string_value(struct expr_literal_t const *expr,
                      char **result, char **error)
{
    return expr_get_value(expr, EXPR_TYPE_STRING, result, error);
}

BOOL
expr_get_integer_value(struct expr_literal_t const *expr,
                       int *result, char **error)
{
    return expr_get_value(expr, EXPR_TYPE_INTEGER, result, error);
}

BOOL
expr_get_boolean_value(struct expr_literal_t const *expr,
                       BOOL *result, char **error)
{
    return expr_get_value(expr, EXPR_TYPE_BOOLEAN, result, error);
}

struct expr_t *
expr_clone(struct expr_t const *expr)
{
    return (*expr->clone)(expr);
}

BOOL
expr_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    if (expr1->kind != expr2->kind || expr1->type != expr2->type) {
        return FALSE;
    }
    else {
        return (*expr1->equal)(expr1, expr2);
    }
}

void
expr_destroy(struct expr_t *expr)
{
    (*expr->free)(expr);
}

struct expr_eval_context_t *
expr_eval_context_create(void) {
    struct expr_eval_context_t *context = (struct expr_eval_context_t *)
        safe_malloc(sizeof(struct expr_eval_context_t));
    context->var_inst = NULL;
    return context;
}

void
expr_eval_context_destroy(struct expr_eval_context_t *THIS) {
    free(THIS);
}
