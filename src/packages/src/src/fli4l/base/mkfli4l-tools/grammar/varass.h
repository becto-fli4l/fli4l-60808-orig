/*****************************************************************************
 *  @file grammar/varass.h
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

#ifndef MKFLI4L_GRAMMAR_VARASS_H_
#define MKFLI4L_GRAMMAR_VARASS_H_

#include "libmkfli4l/defs.h"
#include "libmkfli4l/array.h"
#include "grammar/common.h"
#include "package.h"

/**
 * @name Array indices
 * @{
 */

/**
 * Represents the type of index of an array access. Currently, the following
 * types exist:
 * - top-level variable (e.g. "IP")
 * - structure member (e.g. the ".NET" in "IP.NET")
 * - array member (e.g. the "[1]" in "IP.NET[1]")
 */
enum varass_index_kind_t {
    /**
     * Represents an absolute (numerical) index.
     */
    VARASS_INDEX_ABSOLUTE,
    /**
     * Represents the index of the next free element slot of an array.
     */
    VARASS_INDEX_NEXT_FREE
};

/**
 * Represents the index of an array access.
 */
struct varass_index_t {
    /**
     * The type of index.
     */
    enum varass_index_kind_t kind;
    /**
     * Union of possible members.
     */
    union {
        /**
         * Represents kind == VARASS_INDEX_ABSOLUTE.
         */
        struct {
            /**
             * The index number.
             */
            unsigned number;
        } absolute;

#if 0 /* disabled as empty structures are disallowed */
        /**
         * Represents kind == VARASS_INDEX_NEXT_FREE.
         */
        struct {
        } next_free;
#endif
    } u;
};

/**
 * Creates an absolute array index.
 *
 * @param number
 *  The absolute index.
 * @return
 *  A varass_index_t object. Use varass_index_destroy() to free it after use.
 */
struct varass_index_t *
varass_index_create_absolute(unsigned number);

/**
 * Creates an array index denoting the array's next free element slot.
 *
 * @return
 *  A varass_index_t object. Use varass_index_destroy() to free it after use.
 */
struct varass_index_t *
varass_index_create_next_free(void);

/**
 * Destroys an array index.
 *
 * @param index
 *  The array index to destroy. May be NULL.
 */
void
varass_index_destroy(struct varass_index_t *index);

/**
 * @}
 */

/**
 * @name Assignment targets
 * @{
 */

/**
 * Represents the type of target of an assignment. Currently, the following
 * types exist:
 * - top-level variable (e.g. "IP")
 * - structure member (e.g. the ".NET" in "IP.NET")
 * - array member (e.g. the "[1]" in "IP.NET[1]")
 */
enum varass_target_kind_t {
    /**
     * Represents a (top level) variable.
     */
    VARASS_TARGET_TOP_LEVEL,
    /**
     * Represents a member of a structure.
     */
    VARASS_TARGET_STRUCTURE_MEMBER,
    /**
     * Represents a member of an array.
     */
    VARASS_TARGET_ARRAY_MEMBER
};

/**
 * Represents the target of an assignment.
 */
struct varass_target_t {
    /**
     * The type of target to assign to.
     */
    enum varass_target_kind_t kind;
    /**
     * Union of possible members.
     */
    union {
        /**
         * Represents kind == VARASS_TARGET_TOP_LEVEL.
         */
        struct {
            /**
             * The identifier. NULL for nested assignments.
             */
            struct identifier_t *id;
        } top_level;

        /**
         * Represents kind == VARASS_TARGET_STRUCTURE_MEMBER.
         */
        struct {
            /**
             * The parent target.
             */
            struct varass_target_t *parent;
            /**
             * The member.
             */
            struct identifier_t *member;
        } struct_member;

        /**
         * Represents kind == VARASS_TARGET_ARRAY_MEMBER.
         */
        struct {
            /**
             * The parent target.
             */
            struct varass_target_t *parent;
            /**
             * The index.
             */
            struct varass_index_t *index;
        } array_member;
    } u;
};

/**
 * Creates a top-level assignment target.
 *
 * @param id
 *  The identifier of the variable to assign to.
 * @return
 *  A varass_target_t object. Use varass_target_destroy() to free it after use.
 */
struct varass_target_t *
varass_target_create_top_level(
    struct identifier_t *id
);

/**
 * Creates an assignment target describing a structure member.
 *
 * @param parent
 *  The parent target.
 * @param member
 *  The identifier of the structure member to assign to.
 * @return
 *  A varass_target_t object. Use varass_target_destroy() to free it after use.
 */
struct varass_target_t *
varass_target_create_struct_member(
    struct varass_target_t *parent,
    struct identifier_t *member
);

/**
 * Creates an assignment target describing an array member.
 *
 * @param parent
 *  The parent target.
 * @param index
 *  The index of the array slot to assign to.
 * @return
 *  A varass_target_t object. Use varass_target_destroy() to free it after use.
 */
struct varass_target_t *
varass_target_create_array_member(
    struct varass_target_t *parent,
    struct varass_index_t *index
);

/**
 * Determines if an assignment target is valid.
 *
 * @param target
 *  The assignment target.
 * @return
 *  TRUE if the target is valid, else FALSE.
 */
BOOL
varass_target_is_valid(struct varass_target_t *target);

/**
 * Destroys an assignment target.
 *
 * @param target
 *  The assignment target to destroy. May be NULL.
 */
void
varass_target_destroy(struct varass_target_t *target);

/**
 * @}
 */

/**
 * @name Assignments
 * @{
 */

/**
 * Represents an assignment of a variable.
 */
struct varass_t {
    /**
     * The target.
     */
    struct varass_target_t *target;
    /**
     * The associated value.
     */
    struct qstr_t *value;
    /**
     * The access priority to be used.
     */
    int priority;
    /**
     * The associated comment.
     */
    char *comment;
    /**
     * The location at which this definition is to be found.
     */
    struct location_t *location;
    /**
     * The list of nested assignments.
     */
    struct varass_list_t *nested_assignments;
    /**
     * TRUE if assignment is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a variable assignment.
 *
 * @param target
 *  The target of the assignment
 * @param value
 *  The associated value. May be NULL if the target is a pseudo-target without
 *  being assigned a value.
 * @param priority
 *  The access priority to be used.
 * @param comment
 *  The associated comment.
 * @param location
 *  The location of the definition.
 * @param nested_assignments
 *  A list of nested assignments.
 * @return
 *  A varass_t object. Use varass_destroy() to free it after use.
 */
struct varass_t *
varass_create(
    struct varass_target_t *target,
    struct qstr_t *value,
    int priority,
    char *comment,
    struct location_t *location,
    struct varass_list_t *nested_assignments
);

/**
 * Destroys a variable assignment.
 *
 * @param ass
 *  The variable assignment to destroy. May be NULL.
 */
void
varass_destroy(struct varass_t *ass);

/**
 * @}
 */

/**
 * @name Assignment lists
 * @{
 */

/**
 * Represents a node within a list of variable assignments.
 */
struct varass_list_node_t {
    /**
     * The assignment.
     */
    struct varass_t *assignment;
    /**
     * The previous node (NULL if this is the first node of the list).
     */
    struct varass_list_node_t *prev;
    /**
     * The next node (NULL if this is the last node of the list).
     */
    struct varass_list_node_t *next;
};

/**
 * Represents a list of variable assignments.
 */
struct varass_list_t {
    /**
     * The first node of the list (NULL if the list is empty).
     */
    struct varass_list_node_t *first;
    /**
     * The last node of the list (NULL if the list is empty).
     */
    struct varass_list_node_t *last;
    /**
     * TRUE if all assignments are valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates an empty list of variable assignments.
 *
 * @return
 *  A varass_list_t object. Use varass_list_destroy() to free it after use.
 */
struct varass_list_t *
varass_list_create(void);

/**
 * Appends an assignment to a list of assignments.
 *
 * @param list
 *  The list to append to.
 * @param assignment
 *  The assignment to append.
 * @return
 *  The list.
 */
struct varass_list_t *
varass_list_append(struct varass_list_t *list, struct varass_t *assignment);

/**
 * Destroys a list of variable assignments. All the variable assignments
 * contained therein are also destroyed using varass_destroy().
 *
 * @param list
 *  The list of variable assignments to destroy. May be NULL.
 */
void
varass_list_destroy(struct varass_list_t *list);

/**
 * @}
 */

/**
 * @name Parse tree
 * @{
 */

/**
 * Represents the results of parsing the variable assignment files.
 */
struct varass_parse_tree_t {
    /**
     * TRUE if parse was successful, FALSE otherwise.
     */
    BOOL ok;
    /**
     * The variable assignments parsed.
     */
    array_t *assignments;
};

/**
 * Creates a parse tree structure for variable assignment files.
 *
 * @return
 *  A varass_parse_tree_t object. Use varass_parse_tree_destroy() to free it
 *  after use.
 */
struct varass_parse_tree_t *
varass_parse_tree_create(void);

/**
 * Adds a variable assignment to the parse tree.
 *
 * @param tree
 *  The parse tree.
 * @param ass
 *  The variable assignment to add.
 */
void
varass_parse_tree_add_assignment(
    struct varass_parse_tree_t *tree,
    struct varass_t *ass
);

/**
 * Destroys a parse tree structure for variable assignment files.
 *
 * @param tree
 *  The parse tree to destroy.
 */
void
varass_parse_tree_destroy(struct varass_parse_tree_t *tree);

/**
 * @}
 */

/**
 * The parser's entry point.
 *
 * @param scanner
 *  The scanner to use.
 * @param file
 *  The package file that is to be parsed.
 * @param tree
 *  The parse tree to fill.
 * @param priority
 *  The access priority to be used.
 * @return
 *  Zero if successful, a non-zero value otherwise.
 */
int
varass_parse(
    void *scanner,
    struct package_file_t *file,
    struct varass_parse_tree_t *tree,
    int priority
);

#endif
