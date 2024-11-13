/*****************************************************************************
 *  @file var/core.h
 *  Functions for processing variables (private interface).
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

#ifndef MKFLI4L_VAR_CORE_H_
#define MKFLI4L_VAR_CORE_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>
#include <regex.h>
#include "scope.h"
#include "slot.h"
#include "value.h"

/**
 * The character used as a placeholder in names of array variables.
 */
#define VAR_PLACEHOLDER '%'
/**
 * The character used for separating structure levels.
 */
#define VAR_STRUCTURE_SEPARATOR '_'
/**
 * Regular expression describing variable identifiers.
 * Note: Keep it synchronized with "ID" in vardef_scanner.l and
 * varass_scanner.l!
 */
#define VAR_NAME_REGEX "^[A-Z][A-Z0-9]*(_([A-Z][A-Z0-9]*|%))*$"

/**
 * Type used for checking whether a variable name is valid.
 */
extern struct vartype_t *var_varname_type;

/**
 * Represents a variable definition.
 */
struct var_t {
    /**
     * The common part of all scope members.
     */
    struct var_scope_member_t common;
    /**
     * The variable type.
     */
    struct vartype_t *type;
    /**
     * The scope the variable lives in.
     */
    struct var_scope_t *scope;
    /**
     * The variable template this variable is an instance of. May be NULL.
     */
    struct var_template_t *template;
    /**
     * Number of necessary indices. Level zero denotes a simple variable, level
     * "n" (with n > 0) denotes an n-dimensional array.
     */
    int level;
    /**
     * Reference to the rightmost array bound variable of level "level - 1".
     * If this is not an array (level == 0), this reference is NULL.
     */
    struct var_t *var_n;
    /**
     * Number of array variables referencing this array bound variable.
     */
    int num_var_n_clients;
    /**
     * Array of references to arrays referencing this array bound variable.
     */
    struct var_t **var_n_clients;
    /**
     * Number of instances of this variable.
     */
    int num_instances;
    /**
     * Array of instances of this variable. They are invalidated if this
     * variable is destroyed.
     */
    struct var_instance_t **instances;
    /**
     * Slot of level zero. Contains the variable's value if the variable's
     * level is zero. Otherwise, it points to an array of slots containing
     * slots of the next higher level (level 1) and so on.
     */
    union varslot_t slot;
    /**
     * The variable's priority.
     */
    enum var_priority_t priority;
    /**
     * The properties of the variable.
     */
    struct var_properties_t *props;
    /**
     * Referenced and type-checked variable condition as returned by
     * var_get_condition().
     */
    struct expr_t *condition;
    /**
     * TRUE if condition is valid, else FALSE.
     */
    BOOL cond_valid;
    /**
     * Referenced and type-checked default value expression as returned by
     * var_get_default_value().
     */
    struct expr_t *def_value;
};

/**
 * Registers a variable instance with its variable.
 *
 * @param THIS
 *  The variable.
 * @param instance
 *  The instance.
 */
void
var_register_instance(struct var_t *THIS, struct var_instance_t *instance);

/**
 * Unregisters a variable instance from its variable.
 *
 * @param THIS
 *  The variable.
 * @param instance
 *  The instance.
 */
void
var_unregister_instance(struct var_t *THIS, struct var_instance_t *instance);

/**
 * Destroys a variable.
 *
 * @param THIS
 *  The variable to destroy.
 */
void
var_destroy(struct var_t *THIS);

/**
 * Redefines a variable. This throws away almost everything from the target
 * variable and replaces it with corresponding properties from the source
 * variable, which is destroyed afterwards. The following requirements must be
 * met to make this operation successful:
 *
 * - The target variable must be weak.
 * - The names (and hence the variable level) must be equal.
 * - The scopes must be equal.
 * - The types must be equal.
 * - The priorities of both variables must be equal.
 *
 * Clients and instances are not transferred. It is assumed that the source
 * variable does not have any.
 *
 * @param target
 *  The target variable to redefine.
 * @param source
 *  The source variable. It will be destroyed.
 * @return
 *  TRUE if successful, FALSE otherwise.
 */
BOOL
var_redefine(struct var_t *target, struct var_t *source);

#endif
