/*****************************************************************************
 *  @file vartype/extension.c
 *  Functions for processing variable type extensions.
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
#include <expression.h>
#include <var.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "regex_part.h"
#include "extension.h"

#define MODULE "vartype::extension"

#define VARTYPE_EXT_PREFIX "("
#define VARTYPE_EXT_INFIX ")|("
#define VARTYPE_EXT_SUFFIX ")"

/**
 * Represents a variable type extension represented by a regular expression.
 * Type extensions are hold in a linked list.
 */
struct vartype_extension_t {
    /**
     * The regular expression associated with this type extension, represented
     * by a linked list of expression parts.
     */
    struct vartype_regex_part_t *expr;
    /**
     * The condition associated with this type extension. May be NULL.
     */
    struct expr_t *condition;
    /**
     * TRUE if condition has been already referenced and type-checked.
     */
    BOOL valid;
    /**
     * The error message associated with this type extension.
     */
    char *error_msg;
    /**
     * A comment associated with this type extension.
     */
    char *comment;
    /**
     * The location of this type extension's definition.
     */
    struct location_t *location;
    /**
     * Pointer to the next type extension, if any. May be NULL.
     */
    struct vartype_extension_t *next;
};

struct vartype_extension_t *
vartype_extension_create(
    char const *expr,
    struct expr_t *condition,
    char const *error_msg,
    char const *comment,
    struct location_t *location,
    struct vartype_extension_t **list
)
{
    struct vartype_extension_t *vartype_ext;

    my_assert(expr != NULL);
    my_assert(error_msg != NULL);
    my_assert(comment != NULL);

    vartype_ext = (struct vartype_extension_t *)
        safe_malloc(sizeof(struct vartype_extension_t));

    vartype_ext->expr = vartype_regex_parse(expr);
    vartype_ext->condition = condition ? condition :
        expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));
    vartype_ext->valid = FALSE;
    vartype_ext->error_msg = strsave(error_msg);
    vartype_ext->comment = strsave(comment);
    vartype_ext->location = location;
    vartype_ext->next = NULL;

    while (*list) {
        list = &(*list)->next;
    }
    *list = vartype_ext;

    return vartype_ext;
}

void
vartype_extension_destroy(struct vartype_extension_t *THIS)
{
    if (THIS->next) {
        vartype_extension_destroy(THIS->next);
    }
    vartype_regex_free(THIS->expr);
    expr_destroy(THIS->condition);
    free(THIS->comment);
    free(THIS->error_msg);
    location_destroy(THIS->location);
    free(THIS);
}

/**
 * Validates the condition of a variable type extension. The result is stored
 * in THIS->valid, in addition to being returned.
 *
 * @param THIS
 *  The variable type extension.
 * @param vartype
 *  The variable type this extension belongs to.
 * @return
 *  TRUE if the condition is valid, else FALSE.
 */
static BOOL
vartype_extension_validate_condition(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype
)
{
    if (!THIS->valid) {
        char const *failed_ref;
        struct expr_t *error_expr;

        if (!expr_resolve_variable_expressions(THIS->condition, &failed_ref)) {
            char *expr_string = expr_to_string(THIS->condition);
            char *locstr = location_toString(THIS->location);
            log_info(
                LOG_EXP,
                MODULE ": Resolving variable name '%s' in condition '%s' of type extension '%s' at %s failed.\n",
                expr_string,
                failed_ref,
                vartype->name,
                locstr
            );
            free(locstr);
            free(expr_string);
        }
        else if (!expr_typecheck_expression(THIS->condition, &error_expr)) {
            char *expr_string = expr_to_string(THIS->condition);
            char *error_expr_string = expr_to_string(error_expr);
            char *locstr = location_toString(THIS->location);
            log_info(
                LOG_EXP,
                MODULE ": Expression '%s' being part of condition '%s' of type extension '%s' at %s is not type-conformant.\n",
                error_expr_string,
                expr_string,
                vartype->name,
                locstr
            );
            free(locstr);
            free(error_expr_string);
            free(expr_string);
        }
        else if (expr_get_type(THIS->condition) != EXPR_TYPE_BOOLEAN) {
            char *expr_string = expr_to_string(THIS->condition);
            char *locstr = location_toString(THIS->location);
            log_info(
                LOG_EXP,
                MODULE ": Condition '%s' of type extension '%s' at %s is not a boolean expression.\n",
                expr_string,
                vartype->name,
                locstr
            );
            free(locstr);
            free(expr_string);
        }
        else {
            THIS->valid = TRUE;
        }
    }

    return THIS->valid;
}

char *
vartype_extension_append_regex(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype,
    char *expr,
    BOOL expand
)
{
    int current_len = strlen(expr);

    while (THIS) {
        if (vartype_extension_validate_condition(THIS, vartype)) {
            BOOL enabled = TRUE;
            struct expr_literal_t *cond_result;
            char *error_msg;

            cond_result = expr_evaluate(THIS->condition, NULL);

            if (!expr_get_boolean_value(cond_result, &enabled, &error_msg)) {
                char *locstr = location_toString(THIS->location);
                log_info(
                    LOG_EXP,
                    MODULE ": Cannot evaluate condition of type extension '%s' at %s: %s.\n",
                    vartype->name,
                    locstr,
                    error_msg
                );
                free(locstr);
                free(error_msg);
            }
            else if (enabled) {
                char *regex;
                char *tmp;

                regex = vartype_regex_assemble(THIS->expr, expand);
                if (!regex) {
                    expr_destroy(expr_literal_to_expr(cond_result));
                    free(expr);
                    return NULL;
                }

                current_len += strlen(regex) +
                    sizeof VARTYPE_EXT_PREFIX VARTYPE_EXT_INFIX VARTYPE_EXT_SUFFIX - 1;
                tmp = (char *) safe_malloc(current_len + 1);

                strcpy(tmp, VARTYPE_EXT_PREFIX);
                strcat(tmp, expr);
                free(expr);
                strcat(tmp, VARTYPE_EXT_INFIX);
                strcat(tmp, regex);
                free(regex);
                strcat(tmp, VARTYPE_EXT_SUFFIX);
                expr = tmp;
            }

            expr_destroy(expr_literal_to_expr(cond_result));
        }

        THIS = THIS->next;
    }

    return expr;
}

char *
vartype_extension_assemble_error_message(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype
)
{
    char *result = strsave(vartype->error_msg);
    int current_len = strlen(result);

    while (THIS) {
        if (vartype_extension_validate_condition(THIS, vartype)) {
            BOOL enabled = TRUE;
            struct expr_literal_t *cond_result;
            char *error_msg;

            cond_result = expr_evaluate(THIS->condition, NULL);

            if (!expr_get_boolean_value(cond_result, &enabled, &error_msg)) {
                char *locstr = location_toString(THIS->location);
                log_info(
                    LOG_EXP,
                    MODULE ": Cannot evaluate condition of type extension '%s' at %s: %s.\n",
                    vartype->name,
                    locstr,
                    error_msg
                );
                free(locstr);
                free(error_msg);
            }
            else if (enabled) {
                current_len += strlen(THIS->error_msg);
                result = (char *) safe_realloc(result, current_len + 1);
                strcat(result, THIS->error_msg);
            }

            expr_destroy(expr_literal_to_expr(cond_result));
        }

        THIS = THIS->next;
    }

    return result;
}

char *
vartype_extension_assemble_comment(
    struct vartype_extension_t *THIS,
    struct vartype_t *vartype
)
{
    char *result = strsave(vartype->comment);
    int current_len = strlen(result);

    while (THIS) {
        if (vartype_extension_validate_condition(THIS, vartype)) {
            BOOL enabled = TRUE;
            struct expr_literal_t *cond_result;
            char *error_msg;

            cond_result = expr_evaluate(THIS->condition, NULL);

            if (!expr_get_boolean_value(cond_result, &enabled, &error_msg)) {
                char *locstr = location_toString(THIS->location);
                log_info(
                    LOG_EXP,
                    MODULE ": Cannot evaluate condition of type extension '%s' at %s: %s.\n",
                    vartype->name,
                    locstr,
                    error_msg
                );
                free(locstr);
                free(error_msg);
            }
            else if (enabled) {
                current_len += 1 + strlen(THIS->comment);
                result = (char *) safe_realloc(result, current_len + 1);
                strcat(result, "\n");
                strcat(result, THIS->comment);
            }

            expr_destroy(expr_literal_to_expr(cond_result));
        }

        THIS = THIS->next;
    }

    return result;
}
