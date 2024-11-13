/*****************************************************************************
 *  @file expression/expr_variable.c
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

#include <expression/expr_internal.h>
#include <expression/expr_variable.h>
#include <expression/expr_literal.h>
#include <var.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>

#define MODULE "expression::expr_variable"

/**
 * Describes a (polymorphic) state of a variable expression.
 */
struct expr_variable_state_t {
    /**
     * The state kind.
     */
    enum expr_variable_state_kind_t kind;
    /**
     * Union of possible members.
     */
    union {
        /**
         * Members for kind == EXPR_VARIABLE_STATE_UNRESOLVED.
         */
        struct {
            /**
             * The variable instance referenced by this expression by name.
             */
            char *name;
        } unresolved;
        /**
         * Members for kind == EXPR_VARIABLE_STATE_RESOLVED.
         */
        struct {
            /**
             * The variable instance referenced by this expression.
             */
            struct var_instance_t *instance;
        } resolved;
    } u;
};

/**
 * Represents a variable expression.
 */
struct expr_variable_t {
    /**
     * Common part belonging to every expression. This member must be the first
     * one to allow casting between expr_t and expr_variable_t where necessary.
     */
    struct expr_t expr;
    /**
     * The state of the variable expression.
     */
    struct expr_variable_state_t state;
};

/**
 * Creates an error expression due to failed evaluation of a variable
 * expression.
 *
 * @param scope
 *  The scope of the resulting expression.
 * @param name
 *  The name of the variable instance the evaluation of which has failed.
 * @param msg_tmpl
 *  The message template containing a %s placeholder for the variable name.
 * @return
 *  The error expression.
 */
static struct expr_literal_t *
expr_variable_evaluate_error_by_name(
    struct var_scope_t *scope,
    char const *name,
    char const *msg_tmpl
)
{
    char *msg = safe_sprintf(msg_tmpl, name);
    struct expr_literal_t *result = expr_literal_create_error(scope, msg);
    free(msg);
    return result;
}

/**
 * Creates an error expression due to failed evaluation of a variable
 * expression.
 *
 * @param scope
 *  The scope of the resulting expression.
 * @param instance
 *  The variable instance the evaluation of which has failed.
 * @param msg_tmpl
 *  The message template containing a %s placeholder for the variable name.
 * @return
 *  The error expression.
 */
static struct expr_literal_t *
expr_variable_evaluate_error(
    struct var_scope_t *scope,
    struct var_instance_t *instance,
    char const *msg_tmpl
)
{
    char *name = var_instance_to_string(instance);
    struct expr_literal_t *result
        = expr_variable_evaluate_error_by_name(scope, name, msg_tmpl);
    free(name);
    return result;
}

/**
 * Evaluates a variable expression.
 *
 * @param expr
 *  The expression to evaluate. The caller must ensure that it is a variable
 *  expression.
 * @param context
 *  The evaluation context.
 * @return
 *  A properly typed literal expression representing the value of the
 *  referenced variable.
 */
static struct expr_literal_t *
expr_variable_evaluate(struct expr_t const *expr,
                       struct expr_eval_context_t *context)
{
    struct expr_variable_t const *const varexp
        = (struct expr_variable_t const *) expr;
    struct var_value_t *value;

    struct var_instance_t *inst = varexp->state.u.resolved.instance;
    int level = var_get_level(inst->var);
    if (inst->num_indices < level) {
        if (context &&
            context->var_inst &&
            context->var_inst->num_indices >= level) {
            struct var_instance_t *varinst
                = var_instance_create(inst->var, level,
                                      context->var_inst->indices);
            value = var_instance_try_get_value(varinst);
            var_instance_destroy(varinst);
        }
        else {
            /* This variable instance cannot be evaluated due to missing
             * indices. This typically happens when dependencies of array bound
             * variables are evaluated, and the referenced arrays depend on
             * other arrays. In this case, we replace such an incomplete
             * variable reference with its condition. Recursively, in the
             * absence of cycles, we will eventually evaluate non-array
             * variables. The following example explains this:
             *
             * VAR_%   OPT_X    VAR_N    YESNO "no"
             * VAR_%_X VAR_%    VAR_N    NUMERIC
             *
             * VAR_1='no'
             * VAR_2='yes'
             * VAR_2_X='123'
             * VAR_3='no'
             *
             * Here, VAR_1_X and VAR_3_X are disabled slots as VAR_1 and VAR_3,
             * respectively, are set to 'no'. VAR_N receives the implicitly
             * generated condition (OPT_X || VAR_%). During evaluation, the
             * VAR_% reference is replaced by VAR_%'s condition, OPT_X, which
             * effectively results in evaluating (OPT_X || OPT_X), yielding
             * TRUE if and only if OPT_X is TRUE.
             */
            return expr_evaluate(var_get_condition(inst->var), context);
        }
    }
    else {
        value = var_instance_try_get_value(inst);
    }

    if (value) {
        struct expr_literal_t *result = NULL;
        int int_value;
        BOOL bool_value;

        switch (expr->type) {
        case EXPR_TYPE_STRING :
            result = expr_literal_create_string(expr->scope, value->value);
            break;
        case EXPR_TYPE_INTEGER :
            if (var_value_convert_to_integer(value, &int_value)) {
                result = expr_literal_create_integer(expr->scope, int_value);
            }
            else {
                char *msg_tmpl = safe_sprintf(
                    "converting value '%s' of variable '%%s' to type integer failed",
                    value->value
                );
                result = expr_variable_evaluate_error(expr->scope, inst, msg_tmpl);
                free(msg_tmpl);
            }
            break;
        case EXPR_TYPE_BOOLEAN :
            if (var_value_convert_to_boolean(value, &bool_value)) {
                result = expr_literal_create_boolean(expr->scope, bool_value);
            }
            else {
                char *msg_tmpl = safe_sprintf(
                    "converting value '%s' of variable '%%s' to type boolean failed",
                    value->value
                );
                result = expr_variable_evaluate_error(expr->scope, inst, msg_tmpl);
                free(msg_tmpl);
            }
            break;
        default :
            my_assert(FALSE);
        }

        var_value_destroy(value);
        return result;
    }
    else {
        return expr_variable_evaluate_error(expr->scope, inst, "reading variable '%s' failed");
    }
}

/**
 * Prints a variable expression.
 *
 * @param expr
 *  The expression to print. The caller must ensure that it is a variable
 *  expression.
 * @return
 *  A dynamically allocated string containing the variable name.
 */
static char *
expr_variable_toString(struct expr_t const *expr)
{
    struct expr_variable_t const *const varexp
        = (struct expr_variable_t const *) expr;
    switch (varexp->state.kind) {
    case EXPR_VARIABLE_STATE_RESOLVED :
        return var_instance_to_string(varexp->state.u.resolved.instance);
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        return strdup(varexp->state.u.unresolved.name);
    default :
        return NULL;
    }
}

/**
 * Clones a variable expression.
 *
 * @param expr
 *  The expression to clone. The caller must ensure that it is a variable
 *  expression.
 * @return
 *  The cloned expression.
 */
static struct expr_t *
expr_variable_clone(struct expr_t const *expr)
{
    struct expr_variable_t const *varexp = (struct expr_variable_t const *) expr;
    switch (varexp->state.kind) {
    case EXPR_VARIABLE_STATE_RESOLVED :
        return &expr_variable_create(
            expr->scope,
            var_instance_create(
                varexp->state.u.resolved.instance->var,
                varexp->state.u.resolved.instance->num_indices,
                varexp->state.u.resolved.instance->indices
            )
        )->expr;
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        return &expr_variable_create_by_name(
            expr->scope,
            varexp->state.u.unresolved.name
        )->expr;
    default :
        return NULL;
    }
}

/**
 * Compares two variable expressions.
 *
 * @param expr1
 *  The first expression. The caller must ensure that it is a variable
 *  expression.
 * @param expr2
 *  The second expression. The caller must ensure that it is a variable
 *  expression.
 * @return
 *  TRUE if both expressions are equal, FALSE otherwise.
 */
static BOOL
expr_variable_equal(struct expr_t const *expr1, struct expr_t const *expr2)
{
    struct expr_variable_t const *varexp1 = (struct expr_variable_t const *) expr1;
    struct expr_variable_t const *varexp2 = (struct expr_variable_t const *) expr2;

    /*
     * as an unresolved variable expression has always type EXPR_TYPE_UNKNOWN
     * and a resolved variable expression has always type != EXPR_TYPE_UNKNOWN,
     * expr_equals() is not allowed to call us due to the different types
     */
    my_assert(varexp1->state.kind == varexp2->state.kind);

    switch (varexp1->state.kind) {
    case EXPR_VARIABLE_STATE_RESOLVED :
        return
            varexp1->state.u.resolved.instance->var ==
                varexp2->state.u.resolved.instance->var &&
            varexp1->state.u.resolved.instance->num_indices ==
                varexp2->state.u.resolved.instance->num_indices &&
            memcmp(
                varexp1->state.u.resolved.instance->indices,
                varexp2->state.u.resolved.instance->indices,
                varexp1->state.u.resolved.instance->num_indices * sizeof(unsigned)
            ) == 0;
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        return strcmp(varexp1->state.u.unresolved.name,
                      varexp2->state.u.unresolved.name) == 0;
    default :
        return FALSE;
    }
}

/**
 * Frees a variable expression.
 *
 * @param expr
 *  The expression to free. The caller must ensure that it is a variable
 *  expression.
 */
static void
expr_variable_free(struct expr_t *expr)
{
    struct expr_variable_t const *const varexp
        = (struct expr_variable_t const *) expr;

    switch (varexp->state.kind) {
    case EXPR_VARIABLE_STATE_RESOLVED :
        var_instance_destroy(varexp->state.u.resolved.instance);
        break;
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        free(varexp->state.u.unresolved.name);
        break;
    }

    expr_common_free(expr);
}

struct expr_variable_t *
expr_variable_create(
    struct var_scope_t *scope,
    struct var_instance_t *instance
)
{
    struct expr_variable_t *const varexp
        = (struct expr_variable_t *) safe_malloc(sizeof(struct expr_variable_t));

    expr_common_init(
        &varexp->expr,
        vartype_get_expression_type(var_get_type(instance->var)),
        EXPR_KIND_VARIABLE,
        scope,
        &expr_variable_evaluate,
        &expr_variable_toString,
        &expr_variable_clone,
        &expr_variable_equal,
        &expr_variable_free
    );

    varexp->state.kind = EXPR_VARIABLE_STATE_RESOLVED;
    varexp->state.u.resolved.instance = instance;
    return varexp;
}

struct expr_variable_t *
expr_variable_create_by_name(
    struct var_scope_t *scope,
    char const *name
)
{
    struct expr_variable_t *const varexp
        = (struct expr_variable_t *) safe_malloc(sizeof(struct expr_variable_t));

    expr_common_init(
        &varexp->expr,
        EXPR_TYPE_UNKNOWN,
        EXPR_KIND_VARIABLE,
        scope,
        &expr_variable_evaluate,
        &expr_variable_toString,
        &expr_variable_clone,
        &expr_variable_equal,
        &expr_variable_free
    );

    varexp->state.kind = EXPR_VARIABLE_STATE_UNRESOLVED;
    varexp->state.u.unresolved.name = strdup(name);
    return varexp;
}

struct expr_t *
expr_variable_to_expr(struct expr_variable_t *expr)
{
    return &expr->expr;
}

enum expr_variable_state_kind_t
expr_variable_get_state(struct expr_variable_t *expr)
{
    return expr->state.kind;
}

char const *
expr_variable_get_unresolved_name(struct expr_variable_t *expr)
{
    return expr->state.u.unresolved.name;
}

struct var_instance_t const *
expr_variable_get_resolved_instance(struct expr_variable_t *expr)
{
    return expr->state.u.resolved.instance;
}

BOOL
expr_variable_resolve(struct expr_variable_t *expr)
{
    struct var_instance_t *inst;

    switch (expr->state.kind) {
    case EXPR_VARIABLE_STATE_UNRESOLVED :
        inst = var_instance_create_from_string(expr->expr.scope,
                                               expr->state.u.unresolved.name);
        if (inst) {
            free(expr->state.u.unresolved.name);
            expr->expr.type = vartype_get_expression_type(var_get_type(inst->var));
            expr->state.kind = EXPR_VARIABLE_STATE_RESOLVED;
            expr->state.u.resolved.instance = inst;
            return TRUE;
        }
        else {
            return FALSE;
        }

    case EXPR_VARIABLE_STATE_RESOLVED :
        return TRUE;

    default :
        return FALSE;
    }
}
