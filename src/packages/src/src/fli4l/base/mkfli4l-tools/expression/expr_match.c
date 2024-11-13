/*****************************************************************************
 *  @file expression/expr_match.c
 *  Functions for processing regular expression matching expressions.
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

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <expression/expr_internal.h>
#include <expression/expr_match.h>
#include <expression/expr_literal.h>
#include <vartype.h>
#include <var.h>
#include <package.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MODULE "expression::expr_match"

/**
 * Represents a regular expression matching expression.
 */
struct expr_match_t {
    /**
     * Common part belonging to every expression. This member must be the first
     * one to allow casting between expr_t and expr_variable_t where necessary.
     */
    struct expr_t expr;
    /**
     * The argument to match.
     */
    struct expr_t *arg;
    /**
     * The regular expression to match against.
     */
    struct expr_t *regex;
};

/**
 * Reference to type of EXPR_MATCH_SUBEXP_ARRAY array.
 */
static struct vartype_t *vartype_match_array = NULL;
/**
 * Reference to EXPR_MATCH_SUBEXP_ARRAY array.
 */
static struct var_t *var_match_array = NULL;

/**
 * Returns the type for EXPR_MATCH_SUBEXP_ARRAY array.
 *
 * @return
 *  A pointer to the variable type.
 */
static struct vartype_t *
expr_match_get_array_type(void)
{
    if (!vartype_match_array) {
        vartype_match_array = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
        my_assert(vartype_match_array != NULL);
    }
    return vartype_match_array;
}

/**
 * Returns the EXPR_MATCH_SUBEXP_ARRAY array.
 *
 * @return
 *  A pointer to the variable.
 */
static struct var_t *
expr_match_get_array(struct var_scope_t *scope)
{
    if (!var_match_array) {
        var_match_array
            = var_scope_try_get(scope, EXPR_MATCH_SUBEXP_ARRAY, FALSE);
        if (!var_match_array) {
            struct var_properties_t *props = var_properties_create();
            var_match_array = var_add(
                scope,
                EXPR_MATCH_SUBEXP_ARRAY,
                expr_match_get_array_type(),
                VAR_PRIORITY_SCRIPT_TRANSIENT,
                location_create_internal(PACKAGE_FILE_TYPE_VARDEF),
                props
            );
            my_assert(var_match_array != NULL);
        }
    }
    return var_match_array;
}

/**
 * Evaluates a regular expression matching expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a regular
 *  expression matching expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the result of matching
 *  the argument against the regular expression. Matched subexpressions are
 *  stored in an array called MATCH_%.
 */
static struct expr_literal_t *
expr_match_evaluate(struct expr_t const *expr,
                    struct expr_eval_context_t *context)
{
    char *val;
    char *arg_err;
    struct expr_literal_t *arg_eval;
    struct expr_literal_t *regex_eval;
    char *regex_str;
    struct expr_literal_t *result = NULL;
    struct vartype_t *vartype;
    BOOL b;

    struct expr_match_t const *const matchexp
        = (struct expr_match_t const *) expr;

    arg_eval = expr_evaluate(matchexp->arg, context);
    if (arg_eval->expr.type == EXPR_TYPE_ERROR) {
        return arg_eval;
    }

    regex_eval = expr_evaluate(matchexp->regex, context);
    if (regex_eval->expr.type == EXPR_TYPE_ERROR) {
        expr_destroy(&arg_eval->expr);
        return regex_eval;
    }

    b = expr_get_string_value(arg_eval, &val, &arg_err);
    my_assert_var(b);
    b = expr_get_string_value(regex_eval, &regex_str, &arg_err);
    my_assert_var(b);

    vartype = vartype_add_temporary(
        regex_str,
        location_create_internal(PACKAGE_FILE_TYPE_VARTYPE),
        FALSE
    );
    if (!vartype) {
        char *msg = safe_sprintf(
            "invalid regular expression '%s'",
            regex_str
        );
        result = expr_literal_create_error(expr->scope, msg);
        free(msg);
    }
    else {
        regmatch_t *matches = vartype_match_ext(vartype, val);
        if (matches) {
            struct var_t *match_var = expr_match_get_array(expr->scope);
            struct var_instance_t *match_inst
                = var_instance_create(match_var, 0, NULL);
            if (!var_instance_clear(match_inst, VAR_ACCESS_WEAK,
                    VAR_PRIORITY_SCRIPT_TRANSIENT)) {
                char *msg = safe_sprintf(
                    "cannot clear match array '%s'",
                    var_get_name(match_var)
                );
                result = expr_literal_create_error(expr->scope, msg);
                free(msg);
            }
            else {
                unsigned i;
                regmatch_t const *match = matches + 1;
                struct var_instance_t *inst
                    = var_instance_create(match_var, 1, NULL);

                for (i = 0; match->rm_so != -1; ++i, ++match) {
                    char *substr =
                        substrsave(val + match->rm_so, val + match->rm_eo);
                    BOOL b;

                    inst->indices[0] = i;
                    b = var_instance_set_and_check_value(inst, substr, NULL,
                            VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
                    my_assert_var(b);
                    free(substr);
                }

                var_instance_destroy(inst);
                result = expr_literal_create_boolean(expr->scope, TRUE);
            }

            var_instance_destroy(match_inst);
            free(matches);
        }
        else {
            result = expr_literal_create_boolean(expr->scope, FALSE);
        }

        vartype_remove(vartype);
    }

    expr_destroy(&arg_eval->expr);
    expr_destroy(&regex_eval->expr);
    free(regex_str);
    free(val);
    return result;
}

/**
 * Prints a equality comparison expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a regular
 *  expression matching expression.
 * @return
 *  A dynamically allocated string containing a string representation of the
 *  expression.
 */
static char *
expr_match_toString(struct expr_t const *expr)
{
    struct expr_match_t const *const matchexp
        = (struct expr_match_t const *) expr;
    char *arg_str = expr_to_string(matchexp->arg);
    char *regex_str = expr_to_string(matchexp->regex);
    char *result = safe_sprintf("(%s) =~ (%s)", arg_str, regex_str);
    free(arg_str);
    free(regex_str);
    return result;
}

/**
 * Clones a equality comparison expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a regular
 *  expression matching expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_match_clone(struct expr_t const *expr)
{
    struct expr_match_t const *const matchexp
        = (struct expr_match_t const *) expr;
    return &expr_match_create(
        expr->scope,
        expr_clone(matchexp->arg),
        expr_clone(matchexp->regex)
    )->expr;
}

/**
 * Compares two equality comparison expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a regular
 *  expression matching expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a regular
 *  expression matching expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_match_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_match_t const *matchexp1
        = (struct expr_match_t const *) expr1;
    struct expr_match_t const *matchexp2
        = (struct expr_match_t const *) expr2;
    return expr_equal(matchexp1->arg, matchexp2->arg)
        && expr_equal(matchexp1->regex, matchexp2->regex);
}

/**
 * Frees a equality comparison expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a regular
 *  expression matching expression.
 */
static void
expr_match_free(struct expr_t *expr)
{
    struct expr_match_t *const matchexp = (struct expr_match_t *) expr;
    expr_destroy(matchexp->arg);
    expr_destroy(matchexp->regex);
    expr_common_free(expr);
}

struct expr_match_t *
expr_match_create(
    struct var_scope_t *scope,
    struct expr_t *arg,
    struct expr_t *regex
)
{
    struct expr_match_t *const matchexp
        = (struct expr_match_t *) safe_malloc(sizeof(struct expr_match_t));

    expr_common_init(
        &matchexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_MATCH,
        scope,
        &expr_match_evaluate,
        &expr_match_toString,
        &expr_match_clone,
        &expr_match_equal,
        &expr_match_free
    );

    matchexp->arg = arg;
    matchexp->regex = regex;
    return matchexp;
}

struct expr_t *
expr_match_to_expr(struct expr_match_t *expr)
{
    return &expr->expr;
}

struct expr_t *
expr_match_get_left_arg(struct expr_match_t *expr)
{
    return expr->arg;
}

struct expr_t *
expr_match_get_right_arg(struct expr_match_t *expr)
{
    return expr->regex;
}
