/*****************************************************************************
 *  @file expression/expr_literal.c
 *  Functions for processing literal expressions.
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

#include <expression/expr_internal.h>
#include <expression/expr_literal.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_literal"

/**
 * The quote character used for delimiting string literals.
 */
#define EXPR_LITERAL_STRING_QUOTE "\""
/**
 * The escape character used for escaping quotes in string literals.
 */
#define EXPR_LITERAL_STRING_ESCAPE "\\"

static struct expr_literal_t *
expr_literal_create(struct var_scope_t *scope, int type);

/**
 * Evaluates a literal expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a literal
 *  expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A copy of the literal expression, as literals evaluate to themselves.
 */
static struct expr_literal_t *
expr_literal_evaluate(struct expr_t const *expr,
                      struct expr_eval_context_t *context)
{
    struct expr_literal_t const *literal
        = (struct expr_literal_t const *) expr;
    switch (expr->type) {
    case EXPR_TYPE_ERROR :
        return expr_literal_create_error(expr->scope, literal->value.error_value);
    case EXPR_TYPE_STRING :
        return expr_literal_create_string(expr->scope, literal->value.string_value);
    case EXPR_TYPE_INTEGER :
        return expr_literal_create_integer(expr->scope, literal->value.int_value);
    case EXPR_TYPE_BOOLEAN :
        return expr_literal_create_boolean(expr->scope, literal->value.bool_value);
    default :
        my_assert(FALSE);
        return NULL;
    }
    UNUSED(context);
}

/**
 * Converts a string into a proper string terminal with delimiters and escaped
 * quotes inside.
 *
 * @param s
 *  The string to convert.
 * @return
 *  The converted string.
 */
static char *
expr_literal_quote(char const *s)
{
    char *result = strsave(EXPR_LITERAL_STRING_QUOTE);
    char const *p = s;
    char const *q;
    while (*p) {
        size_t len
            = strcspn(p, EXPR_LITERAL_STRING_QUOTE EXPR_LITERAL_STRING_ESCAPE);
        q = p + len;
        if (*q) {
            char *substr = (char *) safe_malloc(len + 2 + 1);
            strncpy(substr, p, len);
            substr[len] = EXPR_LITERAL_STRING_ESCAPE[0];
            substr[len + 1] = *q;
            substr[len + 2] = '\0';
            result = strcat_save(result, substr);
            free(substr);
            ++q;
        }
        else {
            result = strcat_save(result, p);
        }

        p = q;
    }

    result = strcat_save(result, EXPR_LITERAL_STRING_QUOTE);
    return result;
}

/**
 * Prints a literal expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a literal
 *  expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  literal.
 */
static char *
expr_literal_toString(struct expr_t const *expr)
{
    struct expr_literal_t const *literal
        = (struct expr_literal_t const *) expr;
    switch (expr->type) {
    case EXPR_TYPE_ERROR :
        return strsave(literal->value.error_value);
    case EXPR_TYPE_STRING :
        return expr_literal_quote(literal->value.string_value);
    case EXPR_TYPE_INTEGER :
        return safe_sprintf("%d", literal->value.int_value);
    case EXPR_TYPE_BOOLEAN :
        return literal->value.bool_value
            ? strsave(EXPR_LITERAL_STRING_TRUE)
            : strsave(EXPR_LITERAL_STRING_FALSE);
    default :
        my_assert(FALSE);
        return NULL;
    }
}

/**
 * Clones a literal expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a literal
 *  expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_literal_clone(struct expr_t const *expr)
{
    struct expr_literal_t const *literal
        = (struct expr_literal_t const *) expr;
    struct expr_literal_t *result
        = expr_literal_create(expr->scope, expr_get_type(expr));
    switch (expr->type) {
    case EXPR_TYPE_ERROR :
        result->value.error_value = strsave(literal->value.error_value);
        break;
    case EXPR_TYPE_STRING :
        result->value.string_value = strsave(literal->value.string_value);
        break;
    case EXPR_TYPE_INTEGER :
        result->value.int_value = literal->value.int_value;
        break;
    case EXPR_TYPE_BOOLEAN :
        result->value.bool_value = literal->value.bool_value;
        break;
    default :
        my_assert(FALSE);
        return NULL;
    }

    return &result->expr;
}

/**
 * Compares two literal expressions. They have to be of the same type.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a literal
 *  expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a literal
 *  expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_literal_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_literal_t const *litexp1
        = (struct expr_literal_t const *) expr1;
    struct expr_literal_t const *litexp2
        = (struct expr_literal_t const *) expr2;

    switch (expr1->type) {
    case EXPR_TYPE_ERROR :
        return strcmp(litexp1->value.error_value, litexp2->value.error_value)
            == 0;
    case EXPR_TYPE_STRING :
        return strcmp(litexp1->value.string_value, litexp2->value.string_value)
            == 0;
    case EXPR_TYPE_INTEGER :
        return litexp1->value.int_value == litexp2->value.int_value;
    case EXPR_TYPE_BOOLEAN :
        return litexp1->value.bool_value == litexp2->value.bool_value;
    default :
        my_assert(FALSE);
        return FALSE;
    }
}

/**
 * Frees a literal expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a literal
 *  expression.
 */
static void
expr_literal_free(struct expr_t *expr)
{
    struct expr_literal_t const *literal = (struct expr_literal_t const *) expr;
    if (expr->type == EXPR_TYPE_ERROR) {
        free(literal->value.error_value);
    }
    else if (expr->type == EXPR_TYPE_STRING) {
        free(literal->value.string_value);
    }
    expr_common_free(expr);
}

/**
 * Creates the common part of a literal expression.
 *
 * @param scope
 *  The scope of the literal expression.
 * @param type
 *  The type of the literal expression.
 * @return
 *  The literal expression where only the common part has been initialized.
 */
static struct expr_literal_t *
expr_literal_create(struct var_scope_t *scope, int type)
{
    struct expr_literal_t *const literal
        = (struct expr_literal_t *) safe_malloc(sizeof(struct expr_literal_t));
    expr_common_init(
        &literal->expr,
        type,
        EXPR_KIND_LITERAL,
        scope,
        &expr_literal_evaluate,
        &expr_literal_toString,
        &expr_literal_clone,
        &expr_literal_equal,
        &expr_literal_free
    );
    return literal;
}

struct expr_literal_t *
expr_literal_create_error(struct var_scope_t *scope, char const *value)
{
    struct expr_literal_t *const result
        = expr_literal_create(scope, EXPR_TYPE_ERROR);
    result->value.error_value = strsave(value);
    return result;
}

struct expr_literal_t *
expr_literal_create_string(struct var_scope_t *scope, char const *value)
{
    struct expr_literal_t *const result
        = expr_literal_create(scope, EXPR_TYPE_STRING);
    result->value.string_value = strsave(value);
    return result;
}

struct expr_literal_t *
expr_literal_create_integer(struct var_scope_t *scope, int value)
{
    struct expr_literal_t *const result
        = expr_literal_create(scope, EXPR_TYPE_INTEGER);
    result->value.int_value = value;
    return result;
}

struct expr_literal_t *
expr_literal_create_boolean(struct var_scope_t *scope, BOOL value)
{
    struct expr_literal_t *const result
        = expr_literal_create(scope, EXPR_TYPE_BOOLEAN);
    result->value.bool_value = value;
    return result;
}

void
expr_literal_get_value(struct expr_literal_t const *expr, void *result)
{
    switch (expr->expr.type) {
    case EXPR_TYPE_ERROR :
        *(char **) result = strsave(expr->value.error_value);
        break;
    case EXPR_TYPE_STRING :
        *(char **) result = strsave(expr->value.string_value);
        break;
    case EXPR_TYPE_INTEGER :
        *(int *) result = expr->value.int_value;
        break;
    case EXPR_TYPE_BOOLEAN :
        *(BOOL *) result = expr->value.bool_value;
        break;
    default :
        my_assert(FALSE);
    }
}

struct expr_t *
expr_literal_to_expr(struct expr_literal_t *expr)
{
    return &expr->expr;
}
