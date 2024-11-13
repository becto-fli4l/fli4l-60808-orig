/*****************************************************************************
 *  @file var/scope.h
 *  Functions for processing variable scopes (private interface).
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

#ifndef MKFLI4L_VAR_SCOPE_H_
#define MKFLI4L_VAR_SCOPE_H_

#include <libmkfli4l/defs.h>
#include <var.h>
#include <package.h>
#include <container/hashmap.h>

/**
 * Describes the kind of a member of a scope.
 */
enum var_scope_member_kind_t {
    /**
     * Scope member is a variable.
     */
    VAR_SCOPE_MEMBER_VARIABLE,
    /**
     * Scope member is a variable template.
     */
    VAR_SCOPE_MEMBER_TEMPLATE
};

/**
 * Represents a scope member.
 */
struct var_scope_member_t {
    /**
     * The kind of the scope member.
     */
    enum var_scope_member_kind_t kind;
    /**
     * The name of the scope member.
     */
    char *name;
    /**
     * The location of the scope member.
     */
    struct location_t *location;
    /**
     * Called before a var_scope_member_t object is destroyed.
     */
    void (*destructor)(struct var_scope_member_t *THIS);
};

/**
 * Describes a variable scope.
 */
struct var_scope_t {
    /**
     * The location pointing to the beginning of the scope.
     */
    struct location_t *location;
    /**
     * The parent scope. If NULL, then this is the global scope.
     */
    struct var_scope_t *parent;
    /**
     * The members defined in this scope.
     */
    struct hashmap_t *members;
};

/**
 * Encapsulates a typed scope visitor.
 */
struct var_scope_visitor_adapter_t {
    /**
     * The kind of the scope members the original visitor expects to receive.
     */
    enum var_scope_member_kind_t kind;
    /**
     * The original visitor.
     */
    hashmap_visitor_t visitor;
    /**
     * Additional data for the original visitor.
     */
    void *data;
};

/**
 * Initializes the scope module.
 */
void
var_scope_init(void);

/**
 * Finalizes the scope module.
 */
void
var_scope_fini(void);

/**
 * Initializes a var_scope_member_t object.
 *
 * @param THIS
 *  The var_scope_member_t object to initialize.
 * @param kind
 *  The kind of the scope member.
 * @param name
 *  The name of the scope member.
 * @param location
 *  The location of the scope member.
 * @param destructor
 *  Called before a var_scope_member_t object is destroyed.
 * @return
 *  A pointer to a var_scope_member_t object. Use var_scope_member_destroy()
 *  to free it.
 */
void
var_scope_member_init(
    struct var_scope_member_t *THIS,
    enum var_scope_member_kind_t kind,
    char const *name,
    struct location_t *location,
    void (*destructor)(struct var_scope_member_t *THIS)
);

/**
 * Finalizes a var_scope_member_t object.
 *
 * @param THIS
 *  The var_scope_member_t object to finalize.
 */
void
var_scope_member_fini(struct var_scope_member_t *THIS);

/**
 * Adds an object to a scope.
 *
 * @param THIS
 *  The scope.
 * @param member
 *  The var_scope_member_t object to add.
 * @param old_member
 *  Points to an already existing member with the same name (see below).
 * @return
 *  TRUE if adding the object succeeded, else FALSE. Adding an object may fail
 *  if there is already some other object with the same name in the same scope.
 *  In this case, "old_member" is set to point to that old member. If adding
 *  the new member fails, the object pointed to by "member" is destroyed but
 *  the underlying object is not; this has to be done by the caller if
 *  necessary.
 */
BOOL
var_scope_add_member(
    struct var_scope_t *THIS,
    struct var_scope_member_t *member,
    struct var_scope_member_t const **old_member
);

/**
 * Looks up a member in a given scope hierarchy.
 *
 * @param THIS
 *  The scope to search.
 * @param name
 *  The name of the member.
 * @param recursive
 *  If TRUE, parent scopes are also searched.
 * @return
 *  A pointer to the corresponding var_t object or NULL if not found.
 */
struct var_scope_member_t *
var_scope_try_get_member(
    struct var_scope_t *THIS,
    char const *name,
    BOOL recursive
);

/**
 * Removes an object from a scope by name.
 *
 * @param THIS
 *  The scope.
 * @param name
 *  The name of the object.
 * @return
 *  TRUE if the object was found and removed, else FALSE.
 */
BOOL
var_scope_remove_member(
    struct var_scope_t *THIS,
    char const *name
);

/**
 * Creates a scope visitor adapter. It allows to perform a scope lookup using
 * type-agnostic scope functions, and yet to forward only type-conformant
 * members to the original scope visitor.
 *
 * @param kind
 *  The kind of the scope members the original visitor expects to receive.
 * @param visitor
 *  The original visitor.
 * @param data
 *  Additional data for the original visitor.
 * @return
 *  An object of type var_scope_visitor_adapter_t that can be used as
 *  additional data for var_scope_member_visitor().
 */
struct var_scope_visitor_adapter_t *
var_scope_visitor_adapter_create(
    enum var_scope_member_kind_t kind,
    hashmap_visitor_t visitor,
    void *data
);

/**
 * Destroys a scope visitor adapter.
 *
 * @param THIS
 *  The scope visitor adapter to destroy.
 */
void
var_scope_visitor_adapter_destroy(struct var_scope_visitor_adapter_t *THIS);

/**
 * A type-agnostic scope visitor delegating to a typed scope visitor.
 *
 * @param entry
 *  The scope member being visited.
 * @param data
 *  Additional data. Must point to an object of type
 *   var_scope_visitor_adapter_t.
 * @return
 *  TRUE if the scope member does not conform to the original visitor's type
 *  requirements, else the result of applying the original visitor to the
 *  scope member.
 */
BOOL
var_scope_member_visitor(void *entry, void *data);

/**
 * Iterates over all members in a scope and calls a visitor on them. If the
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
var_scope_foreach_member(
    struct var_scope_t *THIS,
    BOOL recursive,
    hashmap_visitor_t visitor,
    void *data
);

#endif
