/*****************************************************************************
 *  @file var/template.h
 *  Functions for processing variable templates (private interface).
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

#ifndef MKFLI4L_VAR_TEMPLATE_H_
#define MKFLI4L_VAR_TEMPLATE_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>
#include <var.h>
#include <regex.h>
#include "scope.h"
#include "slot.h"

/**
 * Represents a variable template definition. A variable template can be used
 * to generate variables sharing identical naming requirements. The location
 * of such generated variables is copied from the variable template.
 */
struct var_template_t {
    /**
     * The common part of all scope members.
     */
    struct var_scope_member_t common;
    /**
     * The type of the variable template. It will be passed to the variables
     * instantiated from this template.
     */
    struct vartype_t *type;
    /**
     * The scope the variable template lives in. All variables instantiated
     * from this template will be put into this scope.
     */
    struct var_scope_t *scope;
    /**
     * Number of instances of this variable template.
     */
    int num_instances;
    /**
     * Array of instances of this variable template. They are destroyed if this
     * variable template is destroyed.
     */
    struct var_t **instances;
    /**
     * The variable template's priority. It will be passed to the variables
     * instantiated from this template.
     */
    enum var_priority_t priority;
    /**
     * The properties of the variable template. They will be cloned and passed
     * to the variables instantiated from this template.
     */
    struct var_properties_t *props;
    /**
     * Referenced and type-checked variable condition as returned by
     * var_template_get_condition().
     */
    struct expr_t *condition;
    /**
     * TRUE if condition is valid, else FALSE.
     */
    BOOL cond_valid;
    /**
     * The compiled regular expression for template matching.
     */
    regex_t tmpl_regex;
    /**
     * Referenced and type-checked default value expression as returned by
     * var_template_get_default_value().
     */
    struct expr_t *def_value;
};

/**
 * Describes a request for deducing an instantiation from a variable template.
 */
struct var_template_instantiation_request_t {
    /**
     * The original variable name.
     */
    char const *name;
    /**
     * Receives the resulting instance if successful.
     */
    struct var_t *instance;
};

/**
 * Registers a variable template instance with its variable template.
 *
 * @param THIS
 *  The variable template.
 * @param instance
 *  The instance.
 */
void
var_template_register_instance(struct var_template_t *THIS, struct var_t *instance);

/**
 * Unregisters a variable template instance from its variable template.
 *
 * @param THIS
 *  The variable template.
 * @param instance
 *  The instance.
 */
void
var_template_unregister_instance(struct var_template_t *THIS, struct var_t *instance);

/**
 * Tries to find a suitable variable template in a scope to instantiate a
 * variable given its name. The search is non-recursive, i.e. parent scopes
 * are not considered.
 *
 * @param scope
 *  The variable scope.
 * @param name
 *  The name of the variable to create.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  The created variable or NULL if not successful.
 */
struct var_t *
var_template_instantiate_from_string(struct var_scope_t *scope, char const *name, BOOL recursive);

/**
 * Destroys a variable template.
 *
 * @param THIS
 *  The variable template to destroy.
 */
void
var_template_destroy(struct var_template_t *THIS);

#endif
