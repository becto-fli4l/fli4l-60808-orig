/*****************************************************************************
 *  @file var/core.c
 *  Functions for processing variables.
 *
 *  Copyright (c) 2000-2001 - Frank Meyer
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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

#define MODULE "var::core"

/**
 * Name of the variable type used for array bound variables.
 */
#define VAR_TYPE_ARRAY_BOUND VARTYPE_PREDEFINED_UNSIGNED_INTEGER
/**
 * The suffix used for names of array bound variables.
 */
#define VAR_N_SUFFIX 'N'
/**
 * The default value for array bound variables.
 */
#define VAR_N_DEF_VALUE "0"

/**
 * Type used for typing array bounds.
 *
 * @note
 *  Don't use it directly, call var_get_array_bound_type() instead which performs
 *  lazy initialization.
 */
static struct vartype_t *var_array_bound_type = NULL;

/**
 * Returns the type used for typing array bounds.
 */
static struct vartype_t *
var_get_array_bound_type(void)
{
    if (!var_array_bound_type) {
        var_array_bound_type = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
        my_assert(var_array_bound_type);
    }
    return var_array_bound_type;
}

/**
 * Looks up a variable in a given scope hierarchy.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the variable.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @param considerTemplates
 *  If TRUE, check whether a non-existing variable can be instantiated from a
 *  template. This is set to FALSE during instantiation in order to avoid
 *  endless recursion.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 * @note
 *  Templates are only considered if no matching variable has been found in
 *  any scope searched. That means that matching variables in farther scopes
 *  are preferred to matching variable templates in nearer scopes.
 */
static struct var_t *
var_scope_do_try_get(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive,
    BOOL considerTemplates
)
{
    struct var_scope_member_t *const member
        = var_scope_try_get_member(THIS, name, recursive);

    if (member != NULL) {
        if (member->kind == VAR_SCOPE_MEMBER_VARIABLE) {
            return (struct var_t *) member;
        }
        else {
            return NULL;
        }
    }

    if (considerTemplates) {
        struct var_t *const var = var_template_instantiate_from_string(THIS, name, recursive);
        if (var) {
            return var;
        }
    }

    return NULL;
}

struct var_t *
var_scope_try_get(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
)
{
    return var_scope_do_try_get(THIS, name, recursive, TRUE);
}

struct var_t *
var_scope_get(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
)
{
    struct var_t *var = var_scope_try_get(THIS, name, recursive);
    if (var) {
        return var;
    }
    else {
        char *locstr = location_toString(THIS->location);
        log_error(
            MODULE ": Variable '%s' does not exist in scope starting at %s.\n",
            name,
            locstr
        );
        free(locstr);
        return NULL;
    }
}

BOOL
var_scope_foreach(
    struct var_scope_t *THIS,
    BOOL recursive,
    hashmap_visitor_t visitor,
    void *data
)
{
    struct var_scope_visitor_adapter_t *adapter
        = var_scope_visitor_adapter_create(VAR_SCOPE_MEMBER_VARIABLE, visitor, data);
    BOOL result = var_scope_foreach_member(THIS, recursive, var_scope_member_visitor, adapter);
    var_scope_visitor_adapter_destroy(adapter);
    return result;
}

/**
 * Destroys a variable living in a scope.
 *
 * @param THIS
 *  The scope member object encapsulating the variable.
 */
static void
var_scope_destroy_var(struct var_scope_member_t *THIS)
{
    var_destroy((struct var_t *) THIS);
}

/**
 * Adds a variable to a scope. An existing variable may be redefined if
 * - the old one is weak, and
 * - the variables' types are the same, and
 * - the priority of the new variable is less than or equal to the one of the
 *   old variable.
 *
 * @param THIS
 *  The scope.
 * @param var
 *  The variable to add.
 * @return
 *  The variable or NULL if variable could not be added to the scope. Note that
 *  the variable returned need not be the variable passed; this happens when a
 *  weak variable gets redefined. In this case, the variable already part of
 *  the scope is updated and returned, and the variable passed is destroyed.
 */
static struct var_t *
var_scope_add_var(struct var_scope_t *const THIS, struct var_t *const var)
{
    struct var_t *var_old = var_scope_do_try_get(THIS, var->common.name, FALSE, FALSE);
    if (var_old) {
        return var_redefine(var_old, var) ? var_old : NULL;
    }
    else {
        struct var_scope_member_t const *old;
        if (var_scope_add_member(THIS, &var->common, &old)) {
            return var;
        }
        else {
            char *locstr = location_toString(THIS->location);
            char *locstrOrig = location_toString(old->location);
            log_error(
                MODULE ": Redefinition of variable '%s' at %s is not allowed. The original definition is at %s.\n",
                var->common.name, locstr, locstrOrig
            );
            free(locstrOrig);
            free(locstr);
            return NULL;
        }
    }
}

/**
 * Invalidates the cached condition object for a variable and all dependent
 * variables.
 *
 * @param THIS
 *  The variable the condition of which needs to be invalidated.
 */
static void
var_invalidate_condition(struct var_t *THIS)
{
    if (THIS->condition) {
        expr_destroy(THIS->condition);
        THIS->condition = NULL;
        THIS->cond_valid = FALSE;
    }

    if (THIS->var_n) {
        var_invalidate_condition(THIS->var_n);
    }
}

/**
 * Invalidates the cached default value for a variable.
 *
 * @param THIS
 *  The variable the default value of which needs to be invalidated.
 */
static void
var_invalidate_default_value(struct var_t *THIS)
{
    if (THIS->def_value) {
        expr_destroy(THIS->def_value);
        THIS->def_value = NULL;
    }
}

/**
 * Registers a new array variable at an array bound variable which the array
 * depends upon.
 *
 * @param THIS
 *  The array bound variable.
 * @param client
 *  The new array variable.
 */
static void
var_register_client(struct var_t *THIS, struct var_t *client)
{
    THIS->var_n_clients = (struct var_t **) safe_realloc(THIS->var_n_clients,
            (THIS->num_var_n_clients + 1) * sizeof(struct var_t *));
    THIS->var_n_clients[THIS->num_var_n_clients] = client;
    ++THIS->num_var_n_clients;
    var_invalidate_condition(THIS);
}

/**
 * Unregisters an array variable being destroyed at an array bound variable
 * which the array depends upon.
 *
 * @param THIS
 *  The array bound variable.
 * @param client
 *  The array variable being destroyed.
 */
static void
var_unregister_client(struct var_t *THIS, struct var_t *client)
{
    int i;
    for (i = 0; i < THIS->num_var_n_clients; ++i) {
        if (THIS->var_n_clients[i] == client) {
            --THIS->num_var_n_clients;
            memmove(
                &THIS->var_n_clients[i],
                &THIS->var_n_clients[i + 1],
                (THIS->num_var_n_clients - i) * sizeof(struct var_t *)
            );
            THIS->var_n_clients = (struct var_t **) safe_realloc(
                THIS->var_n_clients,
                THIS->num_var_n_clients * sizeof(struct var_t *)
            );
            var_invalidate_condition(THIS);
            break;
        }
    }
}

static BOOL
var_check_array_bound(
    struct var_t *var_n,
    char const *client_name,
    struct var_properties_t const *props,
    enum var_priority_t priority
)
{
    if (var_n->type != var_get_array_bound_type()) {
        log_error(
            MODULE ": Array bound variable '%s' has wrong type '%s', must be '%s'.\n",
            var_n->common.name, vartype_get_name(var_n->type),
            vartype_get_name(var_get_array_bound_type())
        );
        return FALSE;
    }

    if (var_n->priority != priority) {
        log_error(
            MODULE ": Array bound variable '%s' has priority %d while client variable '%s' has priority %d.\n",
            var_n->common.name, var_n->priority, client_name, priority
        );
        return FALSE;
    }

    if (var_n->props->optional && !props->optional) {
        log_error(
            MODULE ": Array bound variable '%s' is optional while array '%s' is not.\n",
            var_n->common.name, client_name
        );
        return FALSE;
    }

    return TRUE;
}

/**
 * Finds or creates an array bound variable storing one or more array bounds
 * (according to its level). A newly and implicitly created array bound
 * variable has the following properties: It is not optional, it has the
 * default value VAR_N_DEF_VALUE (which corresponds to zero), and its enabling
 * condition is FALSE. The latter ensures that an implicitly created array
 * bound variable is enabled only if at least one array using it is enabled.
 *
 * @param scope
 *  The scope where to add the array bound variable to.
 * @param name
 *  The name of the array bound variable.
 * @param client_name
 *  The name of the array bound's client variable.
 * @param priority
 *  The base priority of the array bound's client variable.
 * @param location
 *  The location of the variable definition.
 * @param created
 *  Is set to TRUE if the array bound has been created.
 * @return
 *  A pointer to the associated var_t object or NULL if the variable could
 *  neither be found nor created (e.g. because a variable with the same name
 *  but different properties already exists).
 */
static struct var_t *
var_get_or_add_array_bound(
    struct var_scope_t *scope,
    char const *name,
    char const *client_name,
    struct var_properties_t const *props,
    enum var_priority_t priority,
    struct location_t const *location,
    BOOL *created
)
{
    struct var_t *var_n = var_scope_try_get(scope, name, FALSE);
    if (var_n) {
        if (!var_check_array_bound(var_n, client_name, props, priority)) {
            return NULL;
        }
        else {
            *created = FALSE;
            return var_n;
        }
    }
    else {
        struct location_t *new_location;
        struct var_properties_t *new_props;

        new_props = var_properties_create();
        new_props->optional = FALSE;
        new_props->weak = TRUE;
        var_properties_set_default_value(
            new_props,
            expr_literal_to_expr(
                expr_literal_create_integer(var_get_global_scope(), 0)
            )
        );
        var_properties_set_disabled_value(new_props, VAR_N_DEF_VALUE);

        new_location = location_clone(location);

        var_n = var_add(
            scope,
            name,
            var_get_array_bound_type(),
            priority,
            new_location,
            new_props
        );
        my_assert(var_n);

        *created = TRUE;
        return var_n;
    }
}

/**
 * Creates a variable. If an array variable is to be created, the associated
 * array bound variables are also created if not already done so. For example,
 * creating the variable VBOX_USER_%_SECTION_%_SECDEF_% also creates the
 * array bound variables VBOX_USER_N, VBOX_USER_%_SECTION_N, and
 * VBOX_USER_%_SECTION_%_SECDEF_N (of which the last two ones are array
 * variables, too).
 *
 * @param scope
 *  The scope of the variable. If needed array bound variables are missing,
 *  they are added to this scope.
 * @param name
 *  The name of the variable.
 * @param array_bound_name
 *  The name of the highest level array bound variable. This is only used for
 *  compatibility with old variable definitions. A warning is printed if the
 *  name of the array bound variable passed differs from the computed one.
 * @param type
 *  The type of the variable.
 * @param priority
 *  The base priority of the variable. See var_add() for a description of this
 *  parameter.
 * @param location
 *  The location of the variable definition. Ownership is passed to the
 *  variable.
 * @param props
 *  The properties to use. Ownership is passed to the variable.
 * @return
 *  A pointer to the new var_t object or NULL if some dependent array bound
 *  variable could neither be found nor added.
 */
static struct var_t *
var_create(
    struct var_scope_t *scope,
    char const *name,
    char const *array_bound_name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
)
{
    char const *p;
    struct var_t *var;
    struct var_t *var_added;

    var = (struct var_t *) safe_malloc(sizeof(struct var_t));
    var->type = type;
    var->level = 0;
    var->priority = priority;
    var->scope = scope;
    var->condition = NULL;
    var->cond_valid = FALSE;
    var->def_value = NULL;
    var->template = NULL;

    var->num_var_n_clients = 0;
    var->var_n_clients = NULL;
    var->num_instances = 0;
    var->instances = NULL;

    p = strrchr(name, VAR_PLACEHOLDER);
    if (p != NULL) {
        BOOL created = FALSE;
        char *var_n_name = substrsave(name, p + 1);
        var_n_name[p - name] = VAR_N_SUFFIX;

        if (array_bound_name != NULL) {
            if (strcmp(array_bound_name, var_n_name) != 0) {
                char *locstr = location_toString(location);
                log_warn(
                    MODULE ": Explicit array bound '%s' differs from computed one '%s'. This is deprecated and will not be supported by mkfli4l anymore soon. Please change the reference to the array bound variable accordingly!\n",
                    array_bound_name,
                    var_n_name
                );
                free(locstr);
                free(var_n_name);
                var_n_name = strsave(array_bound_name);
            }
        }

        var->var_n = var_get_or_add_array_bound(
            scope, var_n_name, name, props, priority, location, &created
        );

        if (var->var_n != NULL) {
            p = name;
            while ((p = strchr(p, VAR_PLACEHOLDER)) != NULL) {
                ++var->level;
                ++p;
            }

            if (array_bound_name != NULL) {
                if (var->level != var->var_n->level + 1) {
                    char *locstr = location_toString(location);
                    log_error(
                        MODULE ": Explicit array bound '%s' of '%s' has level %d (and not %d as it should).\n",
                        var_n_name,
                        name,
                        var->var_n->level,
                        var->level - 1
                    );
                    free(locstr);
                    if (created) {
                        var_remove(var->var_n);
                    }
                    free(var_n_name);
                    free(var);
                    return NULL;
                }
            }

            free(var_n_name);
        }
        else {
            free(var_n_name);
            free(var);
            return NULL;
        }
    }
    else {
        if (array_bound_name != NULL) {
            char *locstr = location_toString(location);
            log_error(
                MODULE ": Non-array variable '%s' has an explicit array bound '%s'.\n",
                name,
                array_bound_name
            );
            free(locstr);
            free(var);
            return NULL;
        }
        var->var_n = NULL;
    }

    var_scope_member_init(
        &var->common,
        VAR_SCOPE_MEMBER_VARIABLE,
        name,
        location,
        &var_scope_destroy_var
    );
    var_slot_init(&var->slot, 0, var->level);
    var->props = props;

    if (var->level > 0) {
        var_register_client(var->var_n, var);
    }

    var_added = var_scope_add_var(scope, var);
    if (!var_added) {
        var->props = NULL;
        var->common.location = NULL;
        var_destroy(var);
        return NULL;
    }

    if (var == var_added) {
        char *locstr = location_toString(location);
        log_info(
            LOG_VAR,
            MODULE ": Variable '%s' %sdefined at %s.\n",
            name, props->weak ? "weakly " : "", locstr
        );
        free(locstr);
    }
    else {
        char *locstr = location_toString(location);
        log_info(
            LOG_VAR,
            MODULE ": Variable '%s' redefined at %s.\n",
            name, locstr
        );
        free(locstr);
    }

    return var_added;
}

struct var_t *
var_try_get(char const *name)
{
    struct var_t *var = var_scope_try_get(var_get_global_scope(), name, TRUE);
    if (var) {
        return var;
    }
    else {
        return NULL;
    }
}

struct var_t *
var_get(char const *name)
{
    struct var_t *var = var_try_get(name);
    if (var) {
        return var;
    }
    else {
        log_error(MODULE ": Variable '%s' does not exist.\n", name);
        return NULL;
    }
}

BOOL
var_redefine(struct var_t *target, struct var_t *source)
{
    my_assert(strcmp(target->common.name, source->common.name) == 0);
    my_assert(target->scope == source->scope);

    if (!target->props->weak) {
        char *locstr = location_toString(source->common.location);
        char *locstrOrig = location_toString(target->common.location);
        log_error(
            MODULE ": Redefinition of variable '%s' at %s is not allowed because the original variable is not weak. The original definition is at %s.\n",
            target->common.name, locstr, locstrOrig
        );
        free(locstrOrig);
        free(locstr);
        return FALSE;
    }

    if (target->type != source->type) {
        char *locstr = location_toString(source->common.location);
        char *locstrOrig = location_toString(target->common.location);
        log_error(
            MODULE ": Redefinition of variable '%s' at %s is not allowed because the types differ. The original definition is at %s.\n",
            target->common.name, locstr, locstrOrig
        );
        free(locstrOrig);
        free(locstr);
        return FALSE;
    }

    if (target->priority != source->priority) {
        char *locstr = location_toString(source->common.location);
        char *locstrOrig = location_toString(target->common.location);
        log_error(
            MODULE ": Redefinition of variable '%s' at %s is not allowed because the priority of the new one (%d) is unequal to the priority of the old one (%d). The original definition is at %s.\n",
            target->common.name, locstr, source->priority, target->priority, locstrOrig
        );
        free(locstrOrig);
        free(locstr);
        return FALSE;
    }

    if (var_set_properties(target, source->props)) {
        source->props = NULL;

        location_destroy(target->common.location);
        target->common.location = source->common.location;
        source->common.location = NULL;

        target->priority = source->priority;

        var_destroy(source);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/**
 * Invalidates all variable instances. This is done when a variable is
 * destroyed.
 *
 * @param THIS
 *  The variable.
 */
static void
var_invalidate_instances(struct var_t *THIS)
{
    int i;
    for (i = 0; i < THIS->num_instances; ++i) {
        var_instance_invalidate(THIS->instances[i]);
    }
}

void
var_remove(struct var_t *THIS)
{
    var_scope_remove_member(THIS->scope, THIS->common.name);
}

void
var_destroy(struct var_t *THIS)
{
    int i;

    var_invalidate_condition(THIS);
    var_invalidate_default_value(THIS);
    var_invalidate_instances(THIS);

    if (THIS->var_n != NULL) {
        var_unregister_client(THIS->var_n, THIS);
    }

    for (i = 0; i < THIS->num_var_n_clients; ++i) {
        THIS->var_n_clients[i]->var_n = NULL;
    }

    var_slot_destroy(&THIS->slot, 0, THIS->level);
    if (THIS->template) {
        var_template_unregister_instance(THIS->template, THIS);
    }
    if (THIS->props) {
        var_properties_destroy(THIS->props);
    }
    free(THIS->instances);
    free(THIS->var_n_clients);
    var_scope_member_fini(&THIS->common);
    free(THIS);
}

struct var_t *
var_add(
    struct var_scope_t *scope,
    char const *name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
)
{
    return var_add_with_array_bound(
        scope,
        name,
        NULL,
        type,
        priority,
        location,
        props
    );
}

struct var_t *
var_add_with_array_bound(
    struct var_scope_t *scope,
    char const *name,
    char const *array_bound_name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
)
{
    struct var_t *var;

    my_assert(name);
    my_assert(type);
    my_assert(props);

    if (!vartype_match(var_varname_type, name)) {
        char *locstr = location_toString(location);
        log_error(
            MODULE ": Variable '%s' at %s is not a valid identifier.\n",
            name,
            locstr
        );
        free(locstr);
        return NULL;

    }

    if (!var_properties_check(props, name)) {
        return NULL;
    }

    var = var_create(
        scope, name, array_bound_name, type, priority, location, props
    );
    if (!var) {
        char *locstr = location_toString(location);
        log_error(
            MODULE ": Variable '%s' at %s could not be created.\n",
            name, locstr
        );
        free(locstr);
        return NULL;
    }

    return var;
}

char const *
var_get_name(struct var_t const *THIS)
{
    return THIS->common.name;
}

struct vartype_t *
var_get_type(struct var_t const *THIS)
{
    return THIS->type;
}

enum var_priority_t
var_get_priority(struct var_t const *THIS)
{
    return THIS->priority;
}

struct var_scope_t *
var_get_scope(struct var_t const *THIS)
{
    return THIS->scope;
}

struct var_template_t *
var_get_template(struct var_t const *THIS)
{
    return THIS->template;
}

int
var_get_level(struct var_t const *THIS)
{
    return THIS->level;
}

struct var_properties_t const *
var_get_properties(struct var_t const *THIS)
{
    return THIS->props;
}

BOOL
var_set_properties(struct var_t *THIS, struct var_properties_t *props)
{
    if (!var_properties_check(props, THIS->common.name)) {
        return FALSE;
    }

    /* all clients of an optional variable need also to be optional */
    if (props->optional) {
        int i;
        for (i = 0; i < THIS->num_var_n_clients; ++i) {
            struct var_t const *const client = THIS->var_n_clients[i];
            if (!client->props->optional) {
                log_error(
                    MODULE ": Variable '%s' is optional but client '%s' is not.\n",
                    THIS->common.name, client->common.name
                );
                return FALSE;
            }
        }
    }

    var_properties_destroy(THIS->props);
    THIS->props = props;
    var_invalidate_condition(THIS);
    var_invalidate_default_value(THIS);
    return TRUE;
}

/**
 * Computes a variable's condition if necessary.
 *
 * @param THIS
 *  The variable.
 */
static void
var_compute_condition(struct var_t *THIS)
{
    if (!THIS->condition) {
        char const *failed_ref;
        struct expr_t *error_expr;
        struct expr_t *condition;

        int i;
        struct expr_t *expr_false
            = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), FALSE));
        struct expr_t *expr_true
            = expr_literal_to_expr(expr_literal_create_boolean(var_get_global_scope(), TRUE));

        if (expr_equal(THIS->props->condition, expr_false)) {
            condition = expr_false;
            expr_destroy(expr_true);
        }
        else if (THIS->num_var_n_clients > 0) {
            array_t *expressions = init_array(THIS->num_var_n_clients);
            BOOL true_found = FALSE;
            int num_entries = 0;

            for (i = 0; i < THIS->num_var_n_clients; ++i) {
                DECLARE_ARRAY_ITER(it, expr, struct expr_t);

                struct expr_t *cond = var_get_condition(THIS->var_n_clients[i]);
                if (expr_equal(cond, expr_true)) {
                    true_found = TRUE;
                    break;
                }
                if (expr_equal(cond, expr_false)) {
                    continue;
                }

                ARRAY_ITER_LOOP(it, expressions, expr) {
                    if (expr_equal(cond, expr)) {
                        cond = NULL;
                        break;
                    }
                }

                if (cond) {
                    append_new_elem(expressions, cond);
                    ++num_entries;
                }
            }

            if (true_found) {
                condition = expr_clone(THIS->props->condition);
                expr_destroy(expr_true);
                expr_destroy(expr_false);
            }
            else
            if (num_entries == 0) {
                condition = expr_false;
                expr_destroy(expr_true);
            }
            else {
                struct expr_t *result = NULL;
                struct var_scope_t *global_scope = var_get_global_scope();
                DECLARE_ARRAY_ITER(it, expr, struct expr_t);

                ARRAY_ITER_LOOP(it, expressions, expr) {
                    result = result
                        ? expr_logical_or_to_expr(expr_logical_or_create(global_scope,
                            expr_tobool_to_expr(expr_tobool_create(global_scope, result)),
                            expr_clone(expr)))
                        : expr_clone(expr);
                }

                if (expr_equal(THIS->props->condition, expr_true)) {
                    condition = result;
                }
                else {
                    condition = expr_logical_and_to_expr(expr_logical_and_create(
                        global_scope,
                        expr_clone(THIS->props->condition),
                        result
                    ));
                }
                expr_destroy(expr_true);
                expr_destroy(expr_false);
            }

            free_array(expressions);
        }
        else {
            condition = expr_clone(THIS->props->condition);
            expr_destroy(expr_true);
            expr_destroy(expr_false);
        }

        if (!expr_resolve_variable_expressions(condition, &failed_ref)) {
            char *expr_string = expr_to_string(condition);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Resolving variable name '%s' in condition '%s' of variable '%s' at %s failed.\n",
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
                MODULE ": Expression '%s' being part of condition '%s' of variable '%s' at %s is not type-conformant.\n",
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
                MODULE ": Condition '%s' of variable '%s' at %s is not a boolean expression.\n",
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
                MODULE ": Variable '%s' has condition '%s'.\n",
                THIS->common.name, expr_string
            );
            free(expr_string);

            THIS->cond_valid = TRUE;
        }

        THIS->condition = condition;
    }
}

struct expr_t *
var_get_condition(struct var_t *THIS)
{
    var_compute_condition(THIS);
    return THIS->condition;
}

BOOL
var_has_valid_condition(struct var_t *THIS)
{
    var_compute_condition(THIS);
    return THIS->cond_valid;
}

/**
 * Computes a variable's default value expression if necessary.
 *
 * @param THIS
 *  The variable.
 */
static void
var_compute_default_value(struct var_t *THIS)
{
    if (!THIS->def_value && THIS->props->def_value) {
        char const *failed_ref;
        struct expr_t *error_expr;
        struct expr_t *def_value = expr_clone(THIS->props->def_value);

        if (!expr_resolve_variable_expressions(def_value, &failed_ref)) {
            char *expr_string = expr_to_string(def_value);
            char *locstr = location_toString(THIS->common.location);
            log_error(
                MODULE ": Resolving variable name '%s' in default value '%s' of variable '%s' at %s failed.\n",
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
                MODULE ": Expression '%s' being part of default value '%s' of variable '%s' at %s is not type-conformant.\n",
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
                MODULE ": Variable '%s' has default value '%s'.\n",
                THIS->common.name, expr_string
            );
            free(expr_string);

            THIS->def_value = def_value;
        }
    }
}

struct expr_t *
var_get_default_value(struct var_t *THIS)
{
    var_compute_default_value(THIS);
    return THIS->def_value;
}

BOOL
var_has_valid_default_value(struct var_t *THIS)
{
    var_compute_default_value(THIS);
    return !THIS->props->def_value || THIS->def_value;
}

struct location_t const *
var_get_location(struct var_t const *THIS)
{
    return THIS->common.location;
}

void
var_register_instance(struct var_t *THIS, struct var_instance_t *instance)
{
    THIS->instances = (struct var_instance_t **) safe_realloc(THIS->instances,
            (THIS->num_instances + 1) * sizeof(struct var_instance_t *));
    THIS->instances[THIS->num_instances] = instance;
    ++THIS->num_instances;
}

void
var_unregister_instance(struct var_t *THIS, struct var_instance_t *instance)
{
    int i;
    for (i = 0; i < THIS->num_instances; ++i) {
        if (THIS->instances[i] == instance) {
            --THIS->num_instances;
            memmove(
                &THIS->instances[i],
                &THIS->instances[i + 1],
                (THIS->num_instances - i) * sizeof(struct var_instance_t *)
            );
            THIS->instances = (struct var_instance_t **) safe_realloc(
                THIS->instances,
                THIS->num_instances * sizeof(struct var_instance_t *)
            );
            break;
        }
    }
}
