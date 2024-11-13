/*****************************************************************************
 *  @file var.h
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

#ifndef MKFLI4L_VAR_H_
#define MKFLI4L_VAR_H_

#include <vartype.h>
#include <package.h>
#include <container/hashmap.h>

struct var_properties_t;
struct var_t;
struct var_template_t;
struct var_scope_t;
struct var_value_iterator_t;
struct expr_t;

/**
 * The access modes are used by the functions var_instance_set_value(),
 * var_instance_set_and_check_value(), var_instance_clear(), and var_write()
 * and describe how to cope with overriding existing values. Note that the
 * higher the access mode, the higher the requirements that have to be met to
 * make the access successful.
 */
enum var_access_mode_t {
    /**
     * Specifies tentative access. Access priority must be at least as high as
     * the slot priority in order to perform a value change. If the access
     * priority is lower, though, the change request is ignored without
     * generating an error.
     *
     * This access mode is used to change array bound variables as these
     * variables may be explicitly in the configuration, such that internal
     * changes have to be ignored in this case.
     */
    VAR_ACCESS_TENTATIVE  = 1,
    /**
     * Specifies weak access. Access priority must be at least as high as the
     * slot priority in order to perform a value change. If the access priority
     * is lower, an error is generated and the slot is not changed.
     *
     * This access mode is used to change local script variables as this mode
     * allows overriding variables previously set.
     */
    VAR_ACCESS_WEAK = 2,
    /**
     * Specifies strong access. Access priority must be higher than the slot
     * priority in order to perform a value change. If the access priority is
     * equal or lower, an error is generated and the slot is not changed.
     *
     * This access mode is used to set variable values coming from the
     * configuration, as multiple changes to the same variable are not allowed.
     */
    VAR_ACCESS_STRONG = 3
};

/**
 * The access priorities are used by the functions var_instance_set_value(),
 * var_instance_set_and_check_value(), var_instance_clear(), and var_write()
 * and describe the "overridability" of values.
 */
enum var_priority_t {
    /**
     * This priority is used for undefined slots.
     */
    VAR_PRIORITY_UNDEFINED = -1,
    /**
     * This priority is used for updating array bound variables when resizing
     * an array.
     */
    VAR_PRIORITY_ARRAY_BOUND = 0,
    /**
     * This priority is used for updating local transient variables within an
     * extended check script.
     */
    VAR_PRIORITY_SCRIPT_TRANSIENT = 1,
    /**
     * This priority is used for updating persistent variables with values
     * originating from the normal configuration.
     */
    VAR_PRIORITY_CONFIGURATION = 2,
    /**
     * This priority is used for updating persistent variables with values
     * originating from the superseding configuration.
     */
    VAR_PRIORITY_SUPERSEDE = 3,
    /**
     * This priority is used for updating persistent variables from within an
     * extended check script.
     */
    VAR_PRIORITY_SCRIPT_PERSISTENT = 4
};

/**
 * @name Variable names.
 * @{
 */

/**
 * Describes a part of a variable name.
 */
enum var_part_kind_t {
    /**
     * Represents a top-level variable part.
     */
    VAR_PART_TOP_LEVEL,
    /**
     * Represents a member being a structure.
     */
    VAR_PART_STRUCTURE,
    /**
     * Represents a member being an array.
     */
    VAR_PART_ARRAY
};

/**
 * Describes a part of a variable or a variable instance. Variables are
 * structured in that they consist of a (named) top-level part and an arbitrary
 * sequence of subsequent structure and array parts. The only difference
 * between parts belonging to a variable or to a variable instance,
 * respectively, is that the latter ones possess a valid index for each array
 * part.
 *
 * Implementer's note: To avoid NULL pointers, the parts are linked from inner
 * to outer, i.e. the last part is the outermost var_part_t object and the
 * first part the innermost one. This also helps the bison parser as it prefers
 * left-recursive rules due to its LALR nature.
 */
struct var_part_t {
    /**
     * The type of the variable part.
     */
    enum var_part_kind_t kind;
    /**
     * Union of possible members.
     */
    union {
        /**
         * Represents kind == VAR_PART_TOP_LEVEL.
         */
        struct {
            /**
             * The top-level name.
             */
            char *name;
        } top_level;

        /**
         * Represents kind == VAR_PART_STRUCTURE.
         */
        struct {
            /**
             * The parent part.
             */
            struct var_part_t *parent;
            /**
             * The name of the member.
             */
            char *name;
        } structure;

        /**
         * Represents kind == VAR_PART_ARRAY.
         */
        struct {
            /**
             * The parent part.
             */
            struct var_part_t *parent;
            /**
             * The one-based array index. Only meaningful for variable
             * instances. For variables, it is set to zero.
             */
            unsigned index;
        } array;
    } u;
};

/**
 * Creates a top-level variable part.
 *
 * @param name
 *  The top-level name.
 * @return
 *  A suitable var_part_t object. Use var_part_destroy() to free it after use.
 */
struct var_part_t *
var_part_create_top_level(char const *name);

/**
 * Creates a variable part describing a member being a structure.
 *
 * @param parent
 *  The parent part.
 * @param name
 *  The name of the member.
 * @return
 *  A suitable var_part_t object. Use var_part_destroy() to free it after use.
 */
struct var_part_t *
var_part_create_struct(struct var_part_t *parent, char const *name);

/**
 * Creates a variable part describing a member being an array.
 *
 * @param parent
 *  The parent part.
 * @param index
 *  The array index. Only meaningful for parts of variable instances. Use zero
 *  for parts belonging to a variable.
 * @return
 *  A suitable var_part_t object. Use var_part_destroy() to free it after use.
 */
struct var_part_t *
var_part_create_array(struct var_part_t *parent, unsigned index);

/**
 * Destroys a variable part and all parent parts it references.
 *
 * @param THIS
 *  The variable part to destroy.
 */
void
var_part_destroy(struct var_part_t *THIS);

/**
 * Converts a var_part_t object to a string (modern style).
 *
 * @param THIS
 *  The variable part to convert.
 * @return
 *  A textual representation of the var_part_t object (modern style).
 */
char *
var_part_to_string(struct var_part_t *THIS);

/**
 * Converts a var_part_t object to a string (classic style).
 *
 * @param THIS
 *  The variable part to convert.
 * @return
 *  A textual representation of the var_part_t object (classic style).
 */
char *
var_part_to_string_classic_style(struct var_part_t *THIS);

/**
 * Creates a variable part from a classic-style string.
 *
 * @param name
 *  The variable name (classic style).
 * @return
 *  A corresponding var_part_t object.
 * @note
 *  There is no var_part_create_from_string(), as it is assumed that
 *  modern-style names only occur in a structured representation, such that the
 *  parser is already capable of directly building the necessary var_part_t
 *  objects.
 */
struct var_part_t *
var_part_create_from_string_classic_style(char const *name);

/**
 * @}
 */

/**
 * @name Variable creation, lookup, and scopes.
 * @{
 */

/**
 * Returns the global variable scope.
 *
 * @return
 *  The global variable scope.
 */
struct var_scope_t *
var_get_global_scope(void);

/**
 * Creates a variable scope.
 *
 * @param location
 *  The location pointing to the beginning of the scope. Ownership is passed to
 *  the scope.
 * @param parent
 *  The parent scope.
 * @return
 *  The new scope being an inner scope of "parent".
 */
struct var_scope_t *
var_scope_create(
    struct location_t *location,
    struct var_scope_t *parent
);

/**
 * Looks up a variable in a given scope hierarchy.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the variable.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 */
struct var_t *
var_scope_try_get(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
);

/**
 * Looks up a variable in a given scope hierarchy as var_scope_try_get(). If
 * the variable cannot be found, an error message is logged.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the variable.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 */
struct var_t *
var_scope_get(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
);

/**
 * Iterates over all variables in a scope and calls a visitor on them. If the
 * visitor returns FALSE at some point, the iteration is aborted and FALSE is
 * returned.
 *
 * @param THIS
 *  The scope.
 * @param recursive
 *  If TRUE, parent scopes are also visited.
 * @param visitor
 *  The visitor.
 * @param data
 *  The data for the visitor.
 * @return
 *  TRUE if all visitor calls returned TRUE, FALSE otherwise.
 */
BOOL
var_scope_foreach(
    struct var_scope_t *THIS,
    BOOL recursive,
    hashmap_visitor_t visitor,
    void *data
);

/**
 * Destroys a variable scope.
 *
 * @param THIS
 *  The scope to destroy.
 */
void
var_scope_destroy(struct var_scope_t *THIS);

/**
 * Looks up a variable in the global scope. If the variable cannot be found,
 * the function tries to instantiate a suitable variable template.
 *
 * @param name
 *  The name of the variable.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 */
struct var_t *
var_try_get(char const *name);

/**
 * Looks up a variable in the global scope. If the variable cannot be found,
 * the function tries to instantiate a suitable variable template. If this
 * fails, an error message is logged.
 *
 * @param name
 *  The name of the variable.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 */
struct var_t *
var_get(char const *name);

/**
 * Adds a new or redefines an existing weak variable.
 *
 * @param scope
 *  The scope where to add the variable to.
 * @param name
 *  The name of the variable.
 * @param type
 *  The type of the variable.
 * @param priority
 *  The base priority of the variable. All slot priorities must be greater than
 *  or equal to this one. This e.g. disallows setting a configuration variable
 *  from a script (VAR_PRIORITY_SCRIPT_TRANSIENT) if the configuration variable
 *  is not set by the configuration.
 * @param location
 *  The location of the variable definition. Ownership is passed to the
 *  variable.
 * @param props
 *  The properties to use. Ownership is passed to the variable.
 * @return
 *  A pointer to the new var_t object or NULL if the variable could not be
 *  created (e.g. because a variable with the same name already exists).
 */
struct var_t *
var_add(
    struct var_scope_t *scope,
    char const *name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
);

/**
 * Adds a new or redefines an existing weak variable where the name of the
 * highest level array bound variable is explicitly given. This function is for
 * compatibility with old variable definitions only, don't use it in new code.
 * If the name of the array bound variable does not match the expectations of
 * the variable subsystem, a warning will be printed.
 *
 * @param scope
 *  See var_get_add().
 * @param name
 *  See var_get_add().
 * @param array_bound_name
 *  The name of the array bound variable. If NULL, this function's behaviour
 *  is the same as that of var_add().
 * @param type
 *  See var_get_add().
 * @param priority
 *  See var_get_add().
 * @param location
 *  See var_get_add().
 * @param props
 *  See var_get_add().
 * @return
 *  See var_get_add().
 */
struct var_t *
var_add_with_array_bound(
    struct var_scope_t *scope,
    char const *name,
    char const *array_bound_name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
);

/**
 * Removes a variable from its scope and destroys it. All existing instances
 * referring to this variable are invalidated. Note that in case of array
 * variables, the array bound variables are _not_ removed and destroyed
 * automatically; this has to be done separately by the caller.
 *
 * @param THIS
 *  The variable to remove and destroy.
 */
void
var_remove(struct var_t *THIS);

/**
 * @}
 */

/**
 * @name Variable template creation, lookup, and scopes.
 * @{
 */

/**
 * Adds a new variable template. A variable template is used for creating a
 * suitable variable if a variable is requested using var_get() or
 * var_try_get() which does not exist. If multiple templates match a given
 * variable's name, it is unspecified which template is used to create the
 * variable in question as no partial ordering on regular expressions has been
 * implemented yet.
 *
 * @param scope
 *  The scope where to add the variable template to. All variables instantiated
 *  from this template will be put into this scope.
 * @param name
 *  The name of the variable template. Any regular expression is allowed, but
 *  the template is only used if the resulting identifier conforms to the
 *  naming rules.
 * @param type
 *  The type of the variable template. It will be passed to the variables
 *  instantiated from this template.
 * @param priority
 *  The base priority of the variable template. It will be passed to the
 *  variables instantiated from this template.
 * @param location
 *  The location of the definition of the variable template. It will be cloned
 *  and passed to the variables instantiated from this template. Ownership is
 *  passed to the variable template.
 * @param props
 *  The properties of the variables instantiated from this template. They will
 *  be cloned and passed to the variables instantiated from this template.
 *  Ownership is passed to the variable template.
 * @return
 *  A pointer to the new var_template_t object or NULL if the variable template
 *  could not be created (e.g. because a variable template with the same name
 *  already exists).
 */
struct var_template_t *
var_template_add(
    struct var_scope_t *scope,
    char const *name,
    struct vartype_t *type,
    enum var_priority_t priority,
    struct location_t *location,
    struct var_properties_t *props
);

/**
 * Looks up a variable template in a given scope hierarchy.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the variable template.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  A pointer to the corresponding var_template_t object or NULL if not found.
 */
struct var_template_t *
var_scope_try_get_template(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
);

/**
 * Looks up a variable template in a given scope hierarchy as
 * var_scope_try_get_template(). If the variable template cannot be found, an
 * error message is logged.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the variable template.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  A pointer to the corresponding var_template_t object or NULL if not found.
 */
struct var_template_t *
var_scope_get_template(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
);

/**
 * Iterates over all variable templates in a scope and calls a visitor on them.
 * If the visitor returns FALSE at some point, the iteration is aborted and
 * FALSE is returned.
 *
 * @param THIS
 *  The scope.
 * @param recursive
 *  If TRUE, parent scopes are also visited.
 * @param visitor
 *  The visitor.
 * @param data
 *  The data for the visitor.
 * @return
 *  TRUE if all visitor calls returned TRUE, FALSE otherwise.
 */
BOOL
var_scope_foreach_template(
    struct var_scope_t *THIS,
    BOOL recursive,
    hashmap_visitor_t visitor,
    void *data
);

/**
 * Looks up a variable template in the global scope.
 *
 * @param name
 *  The name of the variable template.
 * @return
 *  A pointer to the corresponding var_template_t object or NULL if not found.
 */
struct var_template_t *
var_template_try_get(char const *name);

/**
 * Looks up a variable template in the global scope. If the variable template
 * cannot be found, an error message is logged.
 *
 * @param name
 *  The name of the variable template.
 * @return
 *  A pointer to the corresponding var_template_t object or NULL if not found.
 */
struct var_template_t *
var_template_get(char const *name);

/**
 * Removes a variable template from its scope and destroys it. All variables
 * instantiated from this template are also destroyed.
 *
 * @param THIS
 *  The variable template to remove and destroy.
 */
void
var_template_remove(struct var_template_t *THIS);

/**
 * Returns the name of a variable template.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The name of the variable template.
 */
char const *
var_template_get_name(struct var_template_t const *THIS);

/**
 * Returns the type of a variable template.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The type of the variable template.
 */
struct vartype_t *
var_template_get_type(struct var_template_t const *THIS);

/**
 * Returns the base priority of a variable.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The base priority of the variable template.
 */
enum var_priority_t
var_template_get_priority(struct var_template_t const *THIS);

/**
 * Returns the scope of a variable template.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The scope of the variable template.
 */
struct var_scope_t *
var_template_get_scope(struct var_template_t const *THIS);

/**
 * Returns the properties associated with a variable template.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The properties associated with the variable template.
 */
struct var_properties_t const *
var_template_get_properties(struct var_template_t const *THIS);

/**
 * Returns a variable template's condition. Note that this is not the same as
 * the condition stored in the variable template's properties, as a variable
 * template's condition may depend on the conditions of other variables or
 * variable templates.
 *
 * The condition object returned is already referenced and type-checked. It is
 * owned by the variable template and has the same lifetime as the variable
 * template. The caller is not allowed to destroy it. If some sub-expression
 * cannot be referenced or type-checked, or if its type is not boolean, a
 * boolean FALSE expression is returned (and var_template_has_valid_condition()
 * returns FALSE).
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The condition.
 */
struct expr_t *
var_template_get_condition(struct var_template_t *THIS);

/**
 * Returns TRUE if the variable template's condition is valid, i.e. if it has
 * been referenced and type-checked successfully.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  TRUE if the condition is valid, else FALSE.
 */
BOOL
var_template_has_valid_condition(struct var_template_t *THIS);

/**
 * Returns a variable template's default value expression. Note that this is
 * not the same as the default value expression stored in the variable
 * template's properties, as a variable template's default value expression may
 * depend on other variables which are yet unknown at creation time.
 *
 * The expression object returned is already referenced and type-checked. It is
 * owned by the variable template and has the same lifetime as the variable. The caller
 * is not allowed to destroy it. If some sub-expression cannot be referenced or
 * type-checked, NULL is returned.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The condition.
 */
struct expr_t *
var_template_get_default_value(struct var_template_t *THIS);

/**
 * Returns TRUE if the variable template's default value expression is valid,
 * i.e. if it has been referenced and type-checked successfully. Note that TRUE
 * is also returned if no default value expression has been set in the variable
 * template's property set.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  TRUE if the default expression is valid, else FALSE.
 */
BOOL
var_template_has_valid_default_value(struct var_template_t *THIS);

/**
 * Returns the location of a variable template.
 *
 * @param THIS
 *  The variable template.
 * @return
 *  The location.
 */
struct location_t const *
var_template_get_location(struct var_template_t const *THIS);

/**
 * @}
 */

/**
 * @name General properties.
 * @{
 */

/**
 * Returns the name of a variable. If the variable name contains placeholders,
 * they are left unchanged.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The name of the variable.
 */
char const *
var_get_name(struct var_t const *THIS);

/**
 * Returns the type of a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The type of the variable.
 */
struct vartype_t *
var_get_type(struct var_t const *THIS);

/**
 * Returns the base priority of a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The base priority of the variable.
 */
enum var_priority_t
var_get_priority(struct var_t const *THIS);

/**
 * Returns the scope of a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The scope of the variable.
 */
struct var_scope_t *
var_get_scope(struct var_t const *THIS);

/**
 * Returns the template of a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The template the variable has been instantiated from. May be NULL if this
 *  variable is not an instance of a variable template.
 */
struct var_template_t *
var_get_template(struct var_t const *THIS);

/**
 * Returns the level of a variable. The level of a variable denotes the number
 * of indices needed to access a variable slot and corresponds to the number of
 * '%' placeholders in the variable's name.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The level of the variable.
 */
int
var_get_level(struct var_t const *THIS);

/**
 * Returns the properties associated with a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The properties associated with the variable.
 */
struct var_properties_t const *
var_get_properties(struct var_t const *THIS);

/**
 * Updates the properties of a variable.
 *
 * @param THIS
 *  The variable.
 * @param props
 *  The new properties to use. Ownership is passed to the variable if
 *  successful.
 * @return
 *  TRUE if the properties have been changed successfully, FALSE otherwise.
 *  The latter can happen if the properties are not self-consistent, i.e. if
 *  the "optional" property is TRUE and a default value is set at the same
 *  time.
 */
BOOL
var_set_properties(struct var_t *THIS, struct var_properties_t *props);

/**
 * Returns a variable's condition. Note that this is not the same as the
 * condition stored in the variable's properties, as a variable's condition may
 * depend on the conditions of other variables. This is especially true for
 * array bound variables whose condition is a logical AND of its own condition
 * and a logical OR of the conditions of all array variables referencing it.
 *
 * The condition object returned is already referenced and type-checked. It is
 * owned by the variable and has the same lifetime as the variable. The caller
 * is not allowed to destroy it. If some sub-expression cannot be referenced or
 * type-checked, or if its type is not boolean, a boolean FALSE expression is
 * returned (and var_has_valid_condition() returns FALSE).
 *
 * @param THIS
 *  The variable.
 * @return
 *  The condition.
 */
struct expr_t *
var_get_condition(struct var_t *THIS);

/**
 * Returns TRUE if the variable's condition is valid, i.e. if it has been
 * referenced and type-checked successfully.
 *
 * @param THIS
 *  The variable.
 * @return
 *  TRUE if the condition is valid, else FALSE.
 */
BOOL
var_has_valid_condition(struct var_t *THIS);

/**
 * Returns a variable's default value expression. Note that this is not the
 * same as the default value expression stored in the variable's properties, as
 * a variable's default value expression may depend on other variables which
 * are yet unknown at creation time.
 *
 * The expression object returned is already referenced and type-checked. It is
 * owned by the variable and has the same lifetime as the variable. The caller
 * is not allowed to destroy it. If some sub-expression cannot be referenced or
 * type-checked, NULL is returned.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The condition.
 */
struct expr_t *
var_get_default_value(struct var_t *THIS);

/**
 * Returns TRUE if the variable's default value expression is valid, i.e. if it
 * has been referenced and type-checked successfully. Note that TRUE is also
 * returned if no default value expression has been set in the variable's
 * property set.
 *
 * @param THIS
 *  The variable.
 * @return
 *  TRUE if the default expression is valid, else FALSE.
 */
BOOL
var_has_valid_default_value(struct var_t *THIS);

/**
 * Returns the location of a variable.
 *
 * @param THIS
 *  The variable.
 * @return
 *  The location.
 */
struct location_t const *
var_get_location(struct var_t const *THIS);

/**
 * @}
 */

/**
 * @name Additional properties.
 * @{
 */

/**
 * Type of destructor for the data in a var_extra_t object.
 *
 * @param data
 *  A pointer to the data to be freed. May be NULL.
 */
typedef void (*var_extra_free_t)(void *data);

/**
 * Type of clone operation for the data in a var_extra_t object.
 *
 * @param data
 *  A pointer to the data to be cloned. May be NULL.
 */
typedef void *(*var_extra_clone_t)(void *data);

/**
 * Additional data that can be attached to each variable.
 */
struct var_extra_t {
    /**
     * The real value.
     */
    void *data;
    /**
     * The clone operation for the value. If NULL, the data pointer is copied
     * "as is" when the var_extra_t object is cloned.
     */
    var_extra_clone_t clone;
    /**
     * The destructor for the value. If NULL, nothing is done when the
     * var_extra_t object is destroyed.
     */
    var_extra_free_t free;
};

/**
 * Creates an object representing extra variable data.
 *
 * @param data
 *  The associated data.
 * @param clone
 *  The clone operation for the data. May be NULL.
 * @param free
 *  The destructor for the data. May be NULL.
 * @return
 *  A dynamically allocated object of type var_extra_t. Use var_extra_clone()
 *  to clone it and var_extra_destroy() to destroy it.
 */
struct var_extra_t *
var_extra_create(
    void *data,
    var_extra_clone_t clone,
    var_extra_free_t free
);

/**
 * Clones an object representing extra variable data.
 *
 * @param THIS
 *  The var_extra_t object to clone. If "clone" is non-NULL, the clone operation
 *  will be called for "data" (even if "data" is NULL).
 */
struct var_extra_t *
var_extra_clone(struct var_extra_t const *THIS);

/**
 * Destroys an object representing extra variable data.
 *
 * @param THIS
 *  The var_extra_t object to destroy. If "free" is non-NULL, the destructor
 *  will be called for "data" (even if "data" is NULL).
 */
void
var_extra_destroy(struct var_extra_t *THIS);

/**
 * Describes additional properties of a variable that possess a sensible
 * default value.
 */
struct var_properties_t {
    /**
     * A comment associated with the variable.
     */
    char *comment;
    /**
     * TRUE if the variable is optional, i.e. it may be missing some or all
     * values. Defaults to FALSE.
     */
    BOOL optional;
    /**
     * TRUE if the variable is weak, i.e. it may be (re)defined later but only
     * if its type is not changed by the redefinition. Defaults to FALSE.
     */
    BOOL weak;
    /**
     * The default value for this variable. This member is only used if
     * "optional" is set to FALSE. It may be NULL if no default value exists.
     * Defaults to NULL.
     */
    struct expr_t *def_value;
    /**
     * If a disabled variable instance is read, the various variable slot
     * reading functions emit a warning and return NULL. Sometimes, however, it
     * is desirable to return a fixed value in this case (which does not have
     * to match the variable's default value). If this member is set to a valid
     * string, reading a disabled instance of this variable returns this string
     * without a warning. Note that unlike the default value, this disabled
     * value can be set and used even if the variable is marked to be optional.
     */
    char *dis_value;
    /**
     * The condition enabling this variable. May be NULL. If a condition is set
     * and evaluates to FALSE, the associated variable is not written to the
     * image's configuration. Defaults to NULL.
     */
    struct expr_t *condition;
    /**
     * Additional user-defined extra data. May be NULL.
     */
    struct var_extra_t *extra;
};

/**
 * Creates a set of default properties for a new variable. The default
 * properties describe a mandatory (non-optional) variable without a default
 * value and without an associated condition.
 *
 * @return
 *  A property set containing default properties for a new variable.
 */
struct var_properties_t *
var_properties_create(void);

/**
 * Clones a property set.
 *
 * @param THIS
 *  The property set to clone.
 * @return
 *  The cloned properties.
 */
struct var_properties_t *
var_properties_clone(struct var_properties_t const *THIS);

/**
 * Changes the comment of a property set.
 *
 * @param THIS
 *  The property set to modify.
 * @param comment
 *  The comment. Note that the old comment (if any) is free()ed before, so
 *  <code>
 *    var_properties_set_comment(props, props->comment)
 *  </code> will fail.
 */
void
var_properties_set_comment(
    struct var_properties_t *THIS,
    char const *comment
);

/**
 * Changes the default value of a property set.
 *
 * @param THIS
 *  The property set to modify.
 * @param def_value
 *  The new default value. May be NULL to remove any default value associated
 *  with the variable. Note that the old default value (if any) is free()ed
 *  before, so <code>
 *    var_properties_set_default_value(props, props->def_value)
 *  </code> will fail. Ownership of the expression is passed to the property
 *  set.
 */
void
var_properties_set_default_value(
    struct var_properties_t *THIS,
    struct expr_t *def_value
);

/**
 * Changes the disabled value of a property set.
 *
 * @param THIS
 *  The property set to modify.
 * @param dis_value
 *  The new disabled value. Note that the old disabled value (if any) is
 *  free()ed before, so <code>
 *    var_properties_set_disabled_value(props, props->dis_value)
 *  </code> will fail.
 */
void
var_properties_set_disabled_value(
    struct var_properties_t *THIS,
    char const *dis_value
);

/**
 * Changes the condition of a property set.
 *
 * @param THIS
 *  The property set to modify.
 * @param condition
 *  The new condition. Note that the old condition (if any) is freed before,
 *  so <code>
 *    var_properties_set_condition(props, props->condition)
 *  </code> will fail.
 */
void
var_properties_set_condition(
    struct var_properties_t *THIS,
    struct expr_t *condition
);

/**
 * Changes the extra data of a property set.
 *
 * @param THIS
 *  The property set to modify.
 * @param extra
 *  The new extra data. Note that the old extra data (if any) is freed before,
 *  so <code>
 *    var_properties_set_extra(props, props->extra)
 *  </code> will fail.
 */
void
var_properties_set_extra(
    struct var_properties_t *THIS,
    struct var_extra_t *extra
);

/**
 * Checks if a property set is self-consistent. For example, an optional
 * variable may not have a default value.
 *
 * @param THIS
 *  The property set to check.
 * @param name
 *  The name of the variable this set is to be associated with. It is used in
 *  error messages if the property set is incorrect.
 * @return
 *  TRUE if the property set is self-consistent, FALSE otherwise.
 */
BOOL
var_properties_check(struct var_properties_t const *THIS, char const *name);

/**
 * Destroys a property set.
 *
 * @param THIS
 *  The property set to destroy.
 */
void
var_properties_destroy(struct var_properties_t *THIS);

/**
 * @}
 */

/**
 * @name Extra slot data.
 * @{
 */

/**
 * Type of destructor for the data in a var_slot_extra_t object.
 *
 * @param data
 *  A pointer to the data to be freed. May be NULL.
 */
typedef void (*var_slot_extra_free_t)(void *data);

/**
 * Additional data that can be attached to each variable slot separately.
 */
struct var_slot_extra_t {
    /**
     * The real value.
     */
    void *data;
    /**
     * The destructor for the value. If NULL, nothing is done when the slot is
     * destroyed.
     */
    var_slot_extra_free_t free;
};

/**
 * Creates an object representing extra slot data.
 *
 * @param data
 *  The associated data.
 * @param free
 *  The destructor for the data.
 * @return
 *  A dynamically allocated object of type var_slot_extra_t. Use
 *  var_slot_extra_destroy() to destroy it.
 */
struct var_slot_extra_t *
var_slot_extra_create(void *data, var_slot_extra_free_t free);

/**
 * Destroys an object representing extra slot data.
 *
 * @param THIS
 *  The var_slot_extra_t object to destroy. If "free" is non-NULL, the
 *  destructor will be called for "data" (even if "data" is NULL).
 */
void
var_slot_extra_destroy(struct var_slot_extra_t *THIS);

/**
 * @}
 */

/**
 * @name Values.
 * @{
 */

/**
 * Describes kinds of values.
 */
enum var_value_kind_t {
    /**
     * This kind describes no value. It is used to describe unset slots.
     */
    VAR_VALUE_KIND_NONE,
    /**
     * This kind describes a normal variable value that has been explicitly set
     * by the means of one of the variable setters.
     */
    VAR_VALUE_KIND_NORMAL,
    /**
     * This kind describes a variable value that has not been set but derived
     * from a variable's default value property.
     */
    VAR_VALUE_KIND_DEFAULT,
    /**
     * This kind describes a variable value that has not been set but derived
     * from a variable's disabled value property.
     */
    VAR_VALUE_KIND_DISABLED
};

/**
 * Describes a value of a variable slot.
 */
struct var_value_t {
    /**
     * The value.
     */
    char const *value;
    /**
     * The kind of the value.
     */
    enum var_value_kind_t kind;
    /**
     * Extra slot data. May be NULL.
     */
    struct var_slot_extra_t *extra;
    /**
     * The slot priority.
     */
    enum var_priority_t priority;
};

/**
 * Converts a variable value to integer type.
 *
 * @param THIS
 *  The variable value as returned by var_get_value().
 * @param result
 *  Receives the integer value.
 * @return
 *  TRUE if the type conversion was successful, FALSE otherwise.
 */
BOOL
var_value_convert_to_integer(struct var_value_t const *THIS, int *result);

/**
 * Converts a variable value to boolean type.
 *
 * @param THIS
 *  The variable value as returned by var_get_value().
 * @param result
 *  Receives the integer value.
 * @return
 *  TRUE if the type conversion was successful, FALSE otherwise.
 */
BOOL
var_value_convert_to_boolean(struct var_value_t const *THIS, BOOL *result);

/**
 * Destroys a value.
 *
 * @param THIS
 *  The value to destroy.
 */
void
var_value_destroy(struct var_value_t *THIS);

/**
 * @}
 */

/**
 * @name Instances.
 * @{
 */

/**
 * Describes a variable instance. A variable instance is basically a slot
 * address and consists of a variable together with a list of indices. If all
 * indices the variable requires are given, the instance is said to be
 * "complete" (and points to a scalar slot), otherwise it is said to be
 * "partial" (and points to an array slot).
 *
 * If a variable is destroyed all its instances are invalidated by setting
 * their "var" member to NULL. This makes all the var_instance_* functions
 * fail.
 */
struct var_instance_t {
    /**
     * The variable.
     */
    struct var_t *var;
    /**
     * The number of indices in the array.
     */
    int num_indices;
    /**
     * The indices.
     */
    unsigned *indices;
};

/**
 * Creates a variable instance.
 *
 * @param THIS
 *  The variable.
 * @param num_indices
 *  The number of indices to be found in the "indices" array.
 * @param indices
 *  The indices to use. If you provide less entries than the level of the
 *  variable requires, a partial instance is created, otherwise the instance
 *  is complete. May be NULL if to initialize all indices with zero.
 * @return
 *  A variable instance.
 */
struct var_instance_t *
var_instance_create(
    struct var_t *THIS,
    int num_indices,
    unsigned const *indices
);

/**
 * Tries to deduce variable and indices used from the name of a variable
 * instance. For example, V_1_2 returns the variable named V_%_% and the
 * indices 0 and 1 (remember that indices internally count from zero).
 *
 * This function also works for non-array variables; in such a case, the
 * index array returned is NULL.
 *
 * @param scope
 *  The scope where to perform the lookup.
 * @param name
 *  The name of the variable slot.
 * @return
 *  A variable instance or NULL if deduction failed.
 */
struct var_instance_t *
var_instance_create_from_string(struct var_scope_t *scope, char const *name);

/**
 * Returns TRUE if this variable instance is valid, i.e. if it has a non-NULL
 * variable associated. Instances may be invalidated when the underlying
 * variable is destroyed.
 *
 * @param THIS
 *  The variable instance.
 * @return
 *  TRUE if the instance is valid, FALSE otherwise.
 */
BOOL
var_instance_is_valid(struct var_instance_t const *THIS);

/**
 * Returns the string value of a variable instance by replacing the index
 * placeholders by index values. Note that internally, indices are counted from
 * zero, but in variable names visible to the user, they are counted from one.
 *
 * @param THIS
 *  The variable instance.
 * @return
 *  A dynamically allocated string containing the substituted variable name or
 *  NULL if the instance is invalid.
 */
char *
var_instance_to_string(struct var_instance_t const *THIS);

/**
 * Returns TRUE if the variable instance is enabled. A variable instance is
 * enabled if either its variable does not have a condition associated or if
 * its condition evaluates to TRUE. If the evaluation of a condition leads to a
 * cycle (e.g. variable A depends on variable B containing "yes" which itself
 * depends on variable A containing "yes"), the variable instance is taken to
 * be disabled and FALSE is returned.
 *
 * @param THIS
 *  The variable instance.
 * @return
 *  TRUE if the variable instance is not invalid and enabled, FALSE otherwise.
 */
BOOL
var_instance_is_enabled(struct var_instance_t *THIS);

/**
 * Returns the value of a variable slot. Note that if the variable is not set,
 * an error is logged. To prevent this, use var_instance_try_get_value()
 * instead.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot to be read. If the
 *  variable instance addresses an array slot, NULL is returned as one cannot
 *  read a whole array at once.
 * @return
 *  The value of the variable or NULL if the accessed variable slot has not
 *  been set or another error occurs. Use var_value_destroy() to destroy it if
 *  you don't need it anymore.
 */
struct var_value_t *
var_instance_get_value(struct var_instance_t *THIS);

/**
 * Returns the value of a variable slot. If the variable is not set, NULL is
 * returned without generating an error message.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot to be read. If the
 *  variable instance addresses an array slot, NULL is returned as one cannot
 *  read a whole array at once.
 * @return
 *  The value of the variable or NULL if the accessed variable slot has not
 *  been set. Use var_value_destroy() to destroy it if you don't need it
 *  anymore.
 */
struct var_value_t *
var_instance_try_get_value(struct var_instance_t *THIS);

/**
 * Sets the value of a variable slot and checks whether it conforms to the
 * variable's type. Additionally, a slot's extra data may be set.
 *
 * Note that you cannot unset a slot's value or extra data with this function.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot to be written to. If the
 *  variable instance addresses an array slot, NULL is returned as one cannot
 *  write to a whole array at once.
 * @param value
 *  The value. May not be NULL.
 * @param extra
 *  Extra slot data. May be NULL in order to not change a value previously set.
 *  If non-NULL, ownership is passed to the slot.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set, expressed by some VAR_PRIORITY_*
 *  constant. This value is stored in the variable slots and, together with the
 *  access mode, controls whether a slot can be changed or not.
 * @return
 *  TRUE if successful, FALSE otherwise. Note that if the access mode is
 *  VAR_ACCESS_TENTATIVE and the slot access fails due to too low a priority,
 *  TRUE is returned; if a non-NULL var_slot_extra_t object has been provided in
 *  such a case, it is destroyed using var_slot_extra_destroy() to prevent a
 *  memory leak.
 */
BOOL
var_instance_set_and_check_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority
);

/**
 * Sets the value and/or the extra data of a variable slot. If the value is to
 * be changed, the new value is not checked against the variable's type.
 * This function is meant to be used while reading the configuration values as
 * at this time the variable type system is not completely ready (there may
 * exist type extensions which refer to variables being read in).
 *
 * Note that you cannot unset a slot's value or extra data with this function.
 *
 * @param THIS
 *  The variable instance addressing the scalar slot to be written to. If the
 *  variable instance addresses an array slot, NULL is returned as one cannot
 *  write to a whole array at once.
 * @param value
 *  The value. May be NULL in order to not change a value previously set. Note
 *  that using a NULL argument for both 'value' and 'extra' parameters always
 *  causes this function to return TRUE irrespective of the access mode or
 *  priority chosen below.
 * @param extra
 *  Extra slot data. May be NULL in order to not change a value previously set.
 *  If non-NULL, ownership is passed to the slot.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set, expressed by some VAR_PRIORITY_*
 *  constant. This value is stored in the variable slots and, together with the
 *  access mode, controls whether a slot can be changed or not.
 * @return
 *  TRUE if successful, FALSE otherwise. Note that if the access mode is
 *  VAR_ACCESS_TENTATIVE and the slot access fails due to too low a priority,
 *  TRUE is returned; if a non-NULL var_slot_extra_t object has been provided in
 *  such a case, it is destroyed using var_slot_extra_destroy() to prevent a
 *  memory leak.
 */
BOOL
var_instance_set_value(
    struct var_instance_t *THIS,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority
);

/**
 * Returns the next free zero-based index for the (sub-)array pointed to by
 * THIS. For example, given the array (note that we use zero-based indices in
 * this example)
 *
 *   X[0][0] = ...
 *   X[1][0] = ...
 *   X[1][1] = ...
 *
 * we have:
 *
 *   var_instance_get_next_free_index(X, []) == 2
 *     (as 2 is the next free index for the top-level array)
 *   var_instance_get_next_free_index(X, [0]) == 1
 *     (as 1 is the next free index for the subarray addressed by index 0)
 *   var_instance_get_next_free_index(X, [1]) == 2
 *     (as 2 is the next free index for the subarray addressed by index 1)
 *
 * If some index is out of array bounds, zero is returned.
 *
 * @param THIS
 *  The variable instance addressing the (sub-)array slot. Excessive indices
 *  are flagged as an error, as well as exactly that many indices as the
 *  variable's level (as such an address selects an entry slot, not an array
 *  slot).
 * @return
 *  The next free index for the chosen subarray. Note that the dimension of
 *  this index equals to the number of indices passed.
 */
unsigned
var_instance_get_next_free_index(struct var_instance_t *THIS);

/**
 * Clears an array (or sub-array), i.e. makes the array contain no elements
 * afterwards.
 *
 * @param THIS
 *  The variable instance addressing the array slot. If the variable instance
 *  addresses a scalar slot, FALSE is returned as one cannot reset non-array
 *  variables to NULL.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set, expressed by some VAR_PRIORITY_*
 *  constant. This value is stored in the variable slots and, together with the
 *  access mode, controls whether a slot can be changed or not.
 * @return
 *  TRUE if clearing was successful, FALSE otherwise.
 */
BOOL
var_instance_clear(
    struct var_instance_t *THIS,
    enum var_access_mode_t mode,
    enum var_priority_t priority
);

/**
 * Destroys a variable instance.
 *
 * @param THIS
 *  The variable instance to destroy.
 */
void
var_instance_destroy(struct var_instance_t *THIS);

/**
 * @}
 */

/**
 * @name Iterators.
 * @{
 */

/**
 * Creates an iterator iterating over the values of an array variable. This
 * iterator is initially the END iterator.
 *
 * @param THIS
 *  The variable instance addressing the array to iterate over. Its lifetime
 *  need not encompass the lifetime of the iterator, it is used only during
 *  creation.
 * @param all
 *  If TRUE, all values are visited. If FALSE, only non-NULL values are
 *  returned.
 * @return
 *  A var_value_iterator_t object. Use var_value_iterator_destroy() to destroy
 *  it if you don't use it anymore.
 */
struct var_value_iterator_t *
var_value_iterator_create(struct var_instance_t *THIS, BOOL all);

/**
 * Advances the iterator to the next element. If there is no next element, the
 * iterator becomes (or remains, see below) the END iterator. If the iterator
 * is already the END iterator, the iterator will be reset to the first array
 * element if one is available, else it remains the END iterator.
 *
 * @param THIS
 *  The iterator.
 * @return
 *  TRUE if the resulting iterator is not the END iterator and
 *  var_value_iterator_get() will succeed the next time it is called on this
 *  iterator, FALSE otherwise.
 */
BOOL
var_value_iterator_next(struct var_value_iterator_t *THIS);

/**
 * Returns the variable associated with an iterator.
 *
 * @param THIS
 *  The iterator.
 * @return
 *  The associated variable.
 */
struct var_t *
var_value_iterator_get_var(struct var_value_iterator_t *THIS);

/**
 * Returns the indices associated with the array element the iterator points
 * to. If the iterator is the END iterator, NULL is returned.
 *
 * The pointer returned is managed by the iterator. Don't free it. The contents
 * of the array will be overwritten the next time var_value_iterator_next()
 * is called.
 *
 * @param THIS
 *  The iterator.
 * @return
 *  The indices to use in order to select the value the iterator is pointing
 *  to.
 */
unsigned *
var_value_iterator_get_var_indices(struct var_value_iterator_t *THIS);

/**
 * Returns the instance associated with the array element the iterator points
 * to. If the iterator is the END iterator, NULL is returned.
 *
 * The pointer returned is managed by the iterator. Don't free it. The contents
 * of the array will be overwritten the next time var_value_iterator_next()
 * is called.
 *
 * @param THIS
 *  The iterator.
 * @return
 *  The instance to use in order to select the value the iterator is pointing
 *  to.
 */
struct var_instance_t *
var_value_iterator_get_var_instance(struct var_value_iterator_t *THIS);

/**
 * Returns the value the iterator points to without advancing the iterator to
 * the next element. If the iterator is the END iterator, NULL is returned.
 *
 * The value returned is managed by the iterator. Don't destroy it.  The value
 * will become invalid the next time var_value_iterator_next() is called.
 *
 * @param THIS
 *  The iterator.
 * @return
 *  The value the iterator points to.
 */
struct var_value_t *
var_value_iterator_get(struct var_value_iterator_t *THIS);

/**
 * Destroys an iterator. The iterator need not be the END iterator.
 *
 * @param THIS
 *  The iterator to destroy.
 */
void
var_value_iterator_destroy(struct var_value_iterator_t *THIS);

/**
 * @}
 */

/**
 * @name Type checking.
 * @{
 */

/**
 * Checks the values of all variables against their types.
 *
 * @return
 *  TRUE if checking was successful, FALSE otherwise.
 */
BOOL
var_check_values(void);

/**
 * @}
 */

/**
 * @name Module initialization and finalization.
 * @{
 */

/**
 * Initializes this module.
 */
void
var_init_module(void);

/**
 * Finalizes this module.
 */
void
var_fini_module(void);

/**
 * @}
 */

/**
 * @name Wrapper functions.
 * @{
 */

/**
 * Convenience function for reading from a global variable slot. Uses
 * var_instance_create_from_string() and var_instance_get_value().
 *
 * @param name
 *  The name of the variable slot.
 * @return
 *  The value of the variable slot or NULL if the accessed variable slot has not
 *  been set or another error occurs.
 */
char const *
var_read(char const *name);

/**
 * Convenience function for reading from a global variable slot. Uses
 * var_instance_create_from_string() and var_instance_try_get_value().
 *
 * @param name
 *  The name of the variable slot.
 * @return
 *  The value of the variable slot or NULL if the accessed variable slot has not
 *  been set or another error occurs.
 */
char const *
var_try_read(char const *name);

/**
 * Convenience function for writing to a global variable slot. Uses
 * var_instance_create_from_string() and var_instance_set_value() or
 * var_instance_set_and_check_value(), respectively.
 *
 * @param name
 *  The name of the variable slot.
 * @param value
 *  The value. May be NULL in order to not change a value previously set.
 * @param extra
 *  Extra slot data. May be NULL in order to not change a value previously set.
 *  If non-NULL, ownership is passed to the slot.
 * @param mode
 *  The access mode, expressed by some VAR_ACCESS_* constant.
 * @param priority
 *  The priority of the value being set, expressed by some VAR_PRIORITY_*
 *  constant. This value is stored in the variable slots and, together with the
 *  access mode, controls whether a slot can be changed or not.
 * @param check
 *  TRUE if the value is to be checked, FALSE otherwise.
 * @return
 *  TRUE if successful, FALSE otherwise. Note that if the access mode is
 *  VAR_ACCESS_TENTATIVE and the slot access fails due to too low a priority,
 *  TRUE is returned; if a non-NULL var_slot_extra_t object has been provided in
 *  such a case, it is destroyed using var_slot_extra_destroy() to prevent a
 *  memory leak.
 */
BOOL
var_write(
    char const *name,
    char const *value,
    struct var_slot_extra_t *extra,
    enum var_access_mode_t mode,
    enum var_priority_t priority,
    BOOL check
);

/**
 * @}
 */

#endif
