/*****************************************************************************
 *  @file var/scope.c
 *  Functions for processing variable scopes.
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
#include <stdlib.h>
#include "scope.h"

#define MODULE "var::scope"

static struct var_scope_t *global_scope = NULL;

void
var_scope_member_init(
    struct var_scope_member_t *THIS,
    enum var_scope_member_kind_t const kind,
    char const *const name,
    struct location_t *const location,
    void (*const destructor)(struct var_scope_member_t *THIS)
)
{
    THIS->kind = kind;
    THIS->name = strsave(name);
    THIS->location = location;
    THIS->destructor = destructor;
}

void
var_scope_member_fini(struct var_scope_member_t *const THIS)
{
    location_destroy(THIS->location);
    free(THIS->name);
}

struct var_scope_visitor_adapter_t *
var_scope_visitor_adapter_create(
    enum var_scope_member_kind_t const kind,
    hashmap_visitor_t const visitor,
    void *const data
)
{
    struct var_scope_visitor_adapter_t *const THIS
        = (struct var_scope_visitor_adapter_t *) safe_malloc(sizeof(struct var_scope_visitor_adapter_t));
    THIS->kind = kind;
    THIS->visitor = visitor;
    THIS->data = data;
    return THIS;
}

void
var_scope_visitor_adapter_destroy(struct var_scope_visitor_adapter_t *const THIS)
{
    free(THIS);
}

BOOL
var_scope_member_visitor(void *const entry, void *const data)
{
    struct var_scope_member_t *const member
        = (struct var_scope_member_t *) entry;
    struct var_scope_visitor_adapter_t *const adapter
        = (struct var_scope_visitor_adapter_t *) data;
    if (member->kind == adapter->kind) {
        return (*adapter->visitor)(member, adapter->data);
    }
    else {
        return TRUE;
    }
}

struct var_scope_t *
var_get_global_scope(void)
{
    return global_scope;
}

/**
 * Destroys a scope member.
 *
 * @param entry
 *  The scope member to destroy. It is of type void* in order to be able to use
 *  this function as a hash map object destructor.
 */
static void
var_scope_member_destroy_action(void *const entry)
{
    struct var_scope_member_t *const THIS = (struct var_scope_member_t *) entry;
    if (THIS->destructor) {
        (*THIS->destructor)(THIS);
    }
}

struct var_scope_t *
var_scope_create(struct location_t *const location, struct var_scope_t *const parent)
{
    struct var_scope_t *const scope = (struct var_scope_t *)
        safe_malloc(sizeof(struct var_scope_t));
    scope->location = location;
    scope->parent = parent;
    scope->members = hashmap_create(
        offsetof(struct var_scope_member_t, name),
        1024,
        hash_ci_string,
        compare_ci_string_keys,
        var_scope_member_destroy_action
    );
    return scope;
}

BOOL
var_scope_add_member(
    struct var_scope_t *const THIS,
    struct var_scope_member_t *const member,
    struct var_scope_member_t const **const old_member
)
{
    *old_member = var_scope_try_get_member(THIS, member->name, FALSE);
    if (*old_member) {
        return FALSE;
    }
    else {
        hashmap_put(THIS->members, member);
        return TRUE;
    }
}

struct var_scope_member_t *
var_scope_try_get_member(
    struct var_scope_t *const THIS,
    char const *const name,
    BOOL const recursive
)
{
    struct var_scope_member_t *const member
        = (struct var_scope_member_t *) hashmap_get(THIS->members, &name);
    if (member != NULL) {
        return member;
    }
    else {
        return recursive && THIS->parent
            ? var_scope_try_get_member(THIS->parent, name, recursive) : NULL;
    }
}

BOOL
var_scope_remove_member(
    struct var_scope_t *const THIS,
    char const *const name
)
{
    return hashmap_remove(THIS->members, &name);
}

BOOL
var_scope_foreach_member(
    struct var_scope_t *const THIS,
    BOOL const recursive,
    hashmap_visitor_t const visitor,
    void *const data
)
{
    if (!hashmap_foreach(THIS->members, visitor, data)) {
        return FALSE;
    }
    else if (recursive && THIS->parent) {
        return var_scope_foreach_member(THIS->parent, recursive, visitor, data);
    }
    else {
        return TRUE;
    }
}

void
var_scope_destroy(struct var_scope_t *const THIS)
{
    hashmap_destroy(THIS->members);
    location_destroy(THIS->location);
    free(THIS);
}

void
var_scope_init(void)
{
    global_scope = var_scope_create(
        location_create_internal(PACKAGE_FILE_TYPE_CONFIG),
        NULL
    );
}

void
var_scope_fini(void)
{
    var_scope_destroy(global_scope);
    global_scope = NULL;
}
