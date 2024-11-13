/*****************************************************************************
 *  @file var/template.c
 *  Functions for processing variables templates.
 *
 *  Copyright (c) 2015 - fli4l-Team <team@fli4l.de>
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
#include <libmkfli4l/array.h>
#include <expression.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "core.h"
#include "template.h"
#include "inst.h"
#include "scope.h"

#define MODULE "var::template"

struct var_template_t *
var_scope_try_get_template(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
)
{
    struct var_scope_member_t *const member
        = var_scope_try_get_member(THIS, name, recursive);

    if (member != NULL && member->kind == VAR_SCOPE_MEMBER_TEMPLATE) {
        return (struct var_template_t *) member;
    }
    else {
        return NULL;
    }
}

struct var_template_t *
var_scope_get_template(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
)
{
    struct var_template_t *tmpl
        = var_scope_try_get_template(THIS, name, recursive);
    if (tmpl) {
        return tmpl;
    }
    else {
        char *locstr = location_toString(THIS->location);
        log_error(
            MODULE ": Variable template '%s' does not exist in scope starting at %s.\n",
            name,
            locstr
        );
        free(locstr);
        return NULL;
    }
}

BOOL
var_scope_foreach_template(
    struct var_scope_t *THIS,
    BOOL recursive,
    hashmap_visitor_t visitor,
    void *data
)
{
    struct var_scope_visitor_adapter_t *adapter
        = var_scope_visitor_adapter_create(VAR_SCOPE_MEMBER_TEMPLATE, visitor, data);
    BOOL result = var_scope_foreach_member(THIS, recursive, var_scope_member_visitor, adapter);
    var_scope_visitor_adapter_destroy(adapter);
    return result;
}

/**
 * Destroys a variable template living in a scope.
 *
 * @param member
 *  The scope member object encapsulating the variable template.
 */
static void
var_scope_destroy_var_template(struct var_scope_member_t *member)
{
    var_template_destroy((struct var_template_t *) member);
}

/**
 * Adds a variable template to a scope.
 *
 * @param THIS
 *  The scope.
 * @param tmpl
 *  The variable template to add.
 * @return
 *  TRUE if variable template could be added to the scope, else FALSE.
 */
static BOOL
var_scope_add_template(struct var_scope_t *const THIS, struct var_template_t *const tmpl)
{
    struct var_scope_member_t const *old;
    if (var_scope_add_member(THIS, &tmpl->common, &old)) {
        return TRUE;
    }
    else {
        char *locstr = location_toString(THIS->location);
        char *locstrOrig = location_toString(old->location);
        log_error(
            MODULE ": Redefinition of variable template '%s' at %s is not allowed. The original definition is at %s.\n",
            tmpl->common.name, locstr, locstrOrig
        );
        free(locstrOrig);
        free(locstr);
        return FALSE;
    }
}

struct var_template_t *
var_template_try_get(char const *name)
{
    return var_scope_try_get_template(var_get_global_scope(), name, TRUE);
}

struct var_template_t *
var_template_get(char const *name)
{
    struct var_template_t *tmpl = var_template_try_get(name);
    if (tmpl) {
        return tmpl;
    }
    else {
        log_error(MODULE ": Variable template '%s' does not exist.\n", name);
        return NULL;
    }
}

/**
 * Removes all variable template instances. This is done when a variable
 * template is destroyed.
 *
 * @param THIS
 *  The variable template.
 */
static void
var_template_invalidate_instances(struct var_template_t *THIS)
{
    while (THIS->num_instances > 0) {
        var_remove(THIS->instances[0]);
    }
}

void
var_template_remove(struct var_template_t *THIS)
{
    var_scope_remove_member(THIS->scope, THIS->common.name);
}

void
var_template_destroy(struct var_template_t *THIS)
{
    if (THIS->condition) {
        expr_destroy(THIS->condition);
    }
    if (THIS->def_value) {
        expr_destroy(THIS->def_value);
    }

    var_template_invalidate_instances(THIS);

    if (THIS->props) {
        var_properties_destroy(THIS->props);
    }
    regfree(&THIS->tmpl_regex);
    free(THIS->instances);
    var_scope_member_fini(&THIS->common);
    free(THIS);
}

struct var_template_t *
var_template_add(
    struct var_scope_t *scope,
    char const *name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
)
{
    struct var_template_t *tmpl;
    char *regex = NULL;
    BOOL comp_result;

    my_assert(name != NULL);
    my_assert(type != NULL);
    my_assert(props != NULL);

    if (!var_properties_check(props, name)) {
        return NULL;
    }

    tmpl = (struct var_template_t *) safe_malloc(sizeof(struct var_template_t));

    var_scope_member_init(
        &tmpl->common,
        VAR_SCOPE_MEMBER_TEMPLATE,
        name,
        location,
        &var_scope_destroy_var_template
    );

    tmpl->type = type;
    tmpl->priority = priority;
    tmpl->scope = scope;
    tmpl->props = props;
    tmpl->condition = NULL;
    tmpl->cond_valid = FALSE;
    tmpl->def_value = NULL;
    tmpl->num_instances = 0;
    tmpl->instances = NULL;

    regex = strcat_save(regex, "^(");
    regex = strcat_save(regex, name);
    regex = strcat_save(regex, ")$");
    comp_result = regcomp(&tmpl->tmpl_regex, regex, REG_EXTENDED | REG_NEWLINE | REG_NOSUB) == 0;
    my_assert_var(comp_result);
    free(regex);

    if (!var_scope_add_template(scope, tmpl)) {
        tmpl->props = NULL;
        tmpl->common.location = NULL;
        var_template_destroy(tmpl);
        return NULL;
    }
    else {
        char *locstr = location_toString(location);
        log_info(
            LOG_VAR,
            MODULE ": Variable template '%s' defined at %s.\n",
            name, locstr
        );
        free(locstr);
        return tmpl;
    }
}

char const *
var_template_get_name(struct var_template_t const *THIS)
{
    return THIS->common.name;
}

struct vartype_t *
var_template_get_type(struct var_template_t const *THIS)
{
    return THIS->type;
}

enum var_priority_t
var_template_get_priority(struct var_template_t const *THIS)
{
    return THIS->priority;
}

struct var_scope_t *
var_template_get_scope(struct var_template_t const *THIS)
{
    return THIS->scope;
}

struct var_properties_t const *
var_template_get_properties(struct var_template_t const *THIS)
{
    return THIS->props;
}

/**
 * Computes a variable template's condition if necessary.
 *
 * @param THIS
 *  The variable template.
 */
static void
var_template_compute_condition(struct var_template_t *THIS)
{
    if (!THIS->condition) {
        char const *failed_ref;
        struct expr_t *error_expr;
        struct expr_t *condition = expr_clone(THIS->props->condition);

        if (!expr_resolve_variable_expressions(condition, &failed_ref)) {
            char *expr_string = expr_to_string(condition);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Resolving variable name '%s' in condition '%s' of variable template '%s' at %s failed.\n",
                expr_string,
                failed_ref,
                THIS->common.name,
                locstr
            );
            free(locstr);
            free(expr_string);
            expr_destroy(condition);

            condition = expr_literal_to_expr(expr_literal_create_boolean(
                var_get_global_scope(), FALSE
            ));
        }
        else if (!expr_typecheck_expression(condition, &error_expr)) {
            char *expr_string = expr_to_string(condition);
            char *error_expr_string = expr_to_string(error_expr);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Expression '%s' being part of condition '%s' of variable template '%s' at %s is not type-conformant.\n",
                error_expr_string,
                expr_string,
                THIS->common.name,
                locstr
            );
            free(locstr);
            free(error_expr_string);
            free(expr_string);
            expr_destroy(condition);

            condition = expr_literal_to_expr(expr_literal_create_boolean(
                var_get_global_scope(), FALSE
            ));
        }
        else if (expr_get_type(condition) != EXPR_TYPE_BOOLEAN) {
            char *expr_string = expr_to_string(condition);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Condition '%s' of variable template '%s' at %s is not a boolean expression.\n",
                expr_string,
                THIS->common.name,
                locstr
            );
            free(locstr);
            free(expr_string);
            expr_destroy(condition);

            condition = expr_literal_to_expr(expr_literal_create_boolean(
                var_get_global_scope(), FALSE
            ));
        }
        else {
            char *expr_string = expr_to_string(condition);
            log_info(
                LOG_VAR,
                MODULE ": Variable template '%s' has condition '%s'.\n",
                THIS->common.name, expr_string
            );
            free(expr_string);

            THIS->cond_valid = TRUE;
        }

        THIS->condition = condition;
    }
}

struct expr_t *
var_template_get_condition(struct var_template_t *THIS)
{
    var_template_compute_condition(THIS);
    return THIS->condition;
}

BOOL
var_template_has_valid_condition(struct var_template_t *THIS)
{
    var_template_compute_condition(THIS);
    return THIS->cond_valid;
}

/**
 * Computes a variable template's default value expression if necessary.
 *
 * @param THIS
 *  The variable template.
 */
static void
var_template_compute_default_value(struct var_template_t *THIS)
{
    if (!THIS->def_value && THIS->props->def_value) {
        char const *failed_ref;
        struct expr_t *error_expr;
        struct expr_t *def_value = expr_clone(THIS->props->def_value);

        if (!expr_resolve_variable_expressions(def_value, &failed_ref)) {
            char *expr_string = expr_to_string(def_value);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Resolving variable name '%s' in default value '%s' of variable template '%s' at %s failed.\n",
                expr_string,
                failed_ref,
                THIS->common.name,
                locstr
            );
            free(locstr);
            free(expr_string);
            expr_destroy(def_value);
        }
        else if (!expr_typecheck_expression(def_value, &error_expr)) {
            char *expr_string = expr_to_string(def_value);
            char *error_expr_string = expr_to_string(error_expr);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Expression '%s' being part of default value '%s' of variable template '%s' at %s is not type-conformant.\n",
                error_expr_string,
                expr_string,
                THIS->common.name,
                locstr
            );
            free(locstr);
            free(error_expr_string);
            free(expr_string);
            expr_destroy(def_value);
        }
        else {
            char *expr_string = expr_to_string(def_value);
            log_info(
                LOG_VAR,
                MODULE ": Variable template '%s' has default value '%s'.\n",
                THIS->common.name, expr_string
            );
            free(expr_string);

            THIS->def_value = def_value;
        }
    }
}

struct expr_t *
var_template_get_default_value(struct var_template_t *THIS)
{
    var_template_compute_default_value(THIS);
    return THIS->def_value;
}

BOOL
var_template_has_valid_default_value(struct var_template_t *THIS)
{
    var_template_compute_default_value(THIS);
    return !THIS->props->def_value || THIS->def_value;
}

struct location_t const *
var_template_get_location(struct var_template_t const *THIS)
{
    return THIS->common.location;
}

void
var_template_register_instance(struct var_template_t *THIS, struct var_t *instance)
{
    THIS->instances = (struct var_t **) safe_realloc(THIS->instances,
            (THIS->num_instances + 1) * sizeof(struct var_t *));
    THIS->instances[THIS->num_instances] = instance;
    ++THIS->num_instances;
}

void
var_template_unregister_instance(struct var_template_t *THIS, struct var_t *instance)
{
    int i;
    for (i = 0; i < THIS->num_instances; ++i) {
        if (THIS->instances[i] == instance) {
            --THIS->num_instances;
            memmove(
                &THIS->instances[i],
                &THIS->instances[i + 1],
                (THIS->num_instances - i) * sizeof(struct var_t *)
            );
            THIS->instances = (struct var_t **) safe_realloc(
                THIS->instances,
                THIS->num_instances * sizeof(struct var_t *)
            );
            break;
        }
    }
}

/**
 * Instantiates a variable template given a matching variable name.
 *
 * @param THIS
 *  The variable template to instantiate.
 * @param name
 *  The name of the variable to create.
 * @return
 *  The created variable or NULL if not successful.
 */
static struct var_t *
var_template_instantiate(struct var_template_t *THIS, char const *name)
{
    struct location_t *location = location_clone(THIS->common.location);
    struct var_properties_t *props = var_properties_clone(THIS->props);
    struct var_t *const var = var_add(
        THIS->scope, name, THIS->type, THIS->priority, location, props
    );
    if (var) {
        var->template = THIS;
        var_template_register_instance(THIS, var);
        return var;
    }
    else {
        var_properties_destroy(props);
        location_destroy(location);
        return NULL;
    }
}

/**
 * Tries to deduce a template instantiation.
 *
 * @param entry
 *  The variable template to check.
 * @param data
 *  The template instantiation request.
 * @return
 *  TRUE if no instantiation possible with given variable template, FALSE
 *  otherwise. In the latter case, the "instance" member in the instantiation
 *  request is filled with the variable.
 */
static BOOL
var_template_try_deduce_instantiation(void *entry, void *data)
{
    struct var_template_t *const tmpl = (struct var_template_t *) entry;
    struct var_template_instantiation_request_t *const req
        = (struct var_template_instantiation_request_t *) data;

    if (regexec(&tmpl->tmpl_regex, req->name, 0, NULL, 0) == 0) {
        req->instance = var_template_instantiate(tmpl, req->name);
        return FALSE;
    }
    else {
        return TRUE;
    }
}

struct var_t *
var_template_instantiate_from_string(struct var_scope_t *scope, char const *name, BOOL recursive)
{
    struct var_template_instantiation_request_t *const req
        = (struct var_template_instantiation_request_t *)
            safe_malloc(sizeof(struct var_template_instantiation_request_t));

    req->name = name;
    req->instance = NULL;

    if (var_scope_foreach_template(scope, recursive, &var_template_try_deduce_instantiation, req)) {
        free(req);
        return NULL;
    }
    else {
        struct var_t *const instance = req->instance;
        free(req);
        return instance;
    }
}
