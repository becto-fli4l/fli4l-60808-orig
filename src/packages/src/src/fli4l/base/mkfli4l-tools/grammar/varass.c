/*****************************************************************************
 *  @file grammar/varass.c
 *  Functions for scanning/parsing files containing variable assignments.
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "libmkfli4l/str.h"
#include "grammar/common.h"
#include "grammar/varass.h"

struct varass_index_t *
varass_index_create_absolute(unsigned number)
{
    struct varass_index_t *index
        = (struct varass_index_t *) safe_malloc(sizeof(struct varass_index_t));
    index->kind = VARASS_INDEX_ABSOLUTE;
    index->u.absolute.number = number;
    return index;
}

struct varass_index_t *
varass_index_create_next_free(void)
{
    struct varass_index_t *index
        = (struct varass_index_t *) safe_malloc(sizeof(struct varass_index_t));
    index->kind = VARASS_INDEX_NEXT_FREE;
    return index;
}

void
varass_index_destroy(struct varass_index_t *index)
{
    if (index) {
        free(index);
    }
}

struct varass_target_t *
varass_target_create_top_level(
    struct identifier_t *id
)
{
    struct varass_target_t *target
        = (struct varass_target_t *) safe_malloc(sizeof(struct varass_target_t));
    target->kind = VARASS_TARGET_TOP_LEVEL;
    target->u.top_level.id = id;
    return target;
}

struct varass_target_t *
varass_target_create_struct_member(
    struct varass_target_t *parent,
    struct identifier_t *member
)
{
    struct varass_target_t *target
        = (struct varass_target_t *) safe_malloc(sizeof(struct varass_target_t));
    target->kind = VARASS_TARGET_STRUCTURE_MEMBER;
    target->u.struct_member.parent = parent;
    target->u.struct_member.member = member;
    return target;
}

struct varass_target_t *
varass_target_create_array_member(
    struct varass_target_t *parent,
    struct varass_index_t *index
)
{
    struct varass_target_t *target
        = (struct varass_target_t *) safe_malloc(sizeof(struct varass_target_t));
    target->kind = VARASS_TARGET_ARRAY_MEMBER;
    target->u.array_member.parent = parent;
    target->u.array_member.index = index;
    return target;
}

BOOL
varass_target_is_valid(struct varass_target_t *target)
{
    switch (target->kind) {
    case VARASS_TARGET_TOP_LEVEL :
        return !target->u.top_level.id || target->u.top_level.id->valid;
    case VARASS_TARGET_STRUCTURE_MEMBER :
        return target->u.struct_member.member->valid &&
                varass_target_is_valid(target->u.struct_member.parent);
    case VARASS_TARGET_ARRAY_MEMBER :
        return varass_target_is_valid(target->u.array_member.parent);
    default :
        my_assert(FALSE);
        return FALSE;
    }
}

void
varass_target_destroy(struct varass_target_t *target)
{
    if (target) {
        switch (target->kind) {
        case VARASS_TARGET_TOP_LEVEL :
            identifier_destroy(target->u.top_level.id);
            break;
        case VARASS_TARGET_STRUCTURE_MEMBER :
            identifier_destroy(target->u.struct_member.member);
            varass_target_destroy(target->u.struct_member.parent);
            break;
        case VARASS_TARGET_ARRAY_MEMBER :
            varass_target_destroy(target->u.array_member.parent);
            varass_index_destroy(target->u.array_member.index);
            break;
        }
        free(target);
    }
}

struct varass_t *
varass_create(
    struct varass_target_t *target,
    struct qstr_t *value,
    int priority,
    char *comment,
    struct location_t *location,
    struct varass_list_t *nested_assignments
)
{
    struct varass_t *ass
        = (struct varass_t *) safe_malloc(sizeof(struct varass_t));
    ass->target = target;
    ass->value = value;
    ass->priority = priority;
    ass->comment = comment;
    ass->location = location;
    ass->nested_assignments = nested_assignments;
    ass->valid = varass_target_is_valid(target) && (!value || value->valid)
        && nested_assignments->valid;
    return ass;
}

void
varass_destroy(struct varass_t *ass)
{
    if (ass) {
        varass_target_destroy(ass->target);
        qstr_destroy(ass->value);
        free(ass->comment);
        location_destroy(ass->location);
        varass_list_destroy(ass->nested_assignments);
        free(ass);
    }
}

struct varass_list_t *
varass_list_create(void)
{
    struct varass_list_t *list
        = (struct varass_list_t *) safe_malloc(sizeof(struct varass_list_t));
    list->first = NULL;
    list->last = NULL;
    list->valid = TRUE;
    return list;
}

struct varass_list_t *
varass_list_append(struct varass_list_t *list, struct varass_t *assignment)
{
    struct varass_list_node_t *node
        = (struct varass_list_node_t *) safe_malloc(sizeof(struct varass_list_node_t));
    node->prev = list->last;
    node->next = NULL;
    node->assignment = assignment;

    if (list->last) {
        list->last->next = node;
    }
    else {
        list->first = node;
    }
    list->last = node;

    if (!assignment->valid) {
        list->valid = FALSE;
    }

    return list;
}

void
varass_list_destroy(struct varass_list_t *list)
{
    struct varass_list_node_t *node = list->first;
    while (node) {
        struct varass_list_node_t *next = node->next;
        varass_destroy(node->assignment);
        free(node);
        node = next;
    }

    free(list);
}

struct varass_parse_tree_t *
varass_parse_tree_create(void)
{
    struct varass_parse_tree_t *tree = (struct varass_parse_tree_t *)
        malloc(sizeof(struct varass_parse_tree_t));
    tree->ok = TRUE;
    tree->assignments = init_array(1024);
    return tree;
}

void
varass_parse_tree_add_assignment(
    struct varass_parse_tree_t *tree,
    struct varass_t *ass
)
{
    append_new_elem(tree->assignments, ass);
    if (!ass->valid) {
        tree->ok = FALSE;
    }
}

void
varass_parse_tree_destroy(struct varass_parse_tree_t *tree)
{
    ARRAY_ITER(it, tree->assignments, ass, struct varass_t) {
        varass_destroy(ass);
    }
    free_array(tree->assignments);
    free(tree);
}
