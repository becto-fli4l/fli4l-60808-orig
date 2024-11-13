/*****************************************************************************
 *  @file tree_struct.h
 *  Definitions for parse trees for extended check scripts.
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
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

#ifndef LIBMKFLI4L_TREE_STRUCT_H_
#define LIBMKFLI4L_TREE_STRUCT_H_

#include "libmkfli4l/tree.h"

/**
 * Parse node types.
 * @{
 */
/**
 * Describes an inner node (e.g. a mathematical operation).
 */
#define NODE 1
/**
 * Describes a leaf node (e.g. an identifier).
 */
#define LEAF 2
/**
 * @}
 */

#define OP   ln.node.op
#define ARG  ln.node.arg
#define TYPE ln.leaf.type
#define VAL  ln.leaf.value

/**
 * Represents a leaf node of the parse tree.
 */
typedef struct {
    /**
     * The type of the leaf, described by some TYPE_* constant.
     */
    int type;
    /**
     * The value associated with the leaf.
     */
    char *value;
} leaf_t;

/**
 * Represents an inner node of the parse tree.
 */
typedef struct {
    /**
     * The operation associated with this node, described by some constant
     * found in lex.l.
     */
    int op;
    /**
     * Up to three arguments associated with the operation.
     */
    elem_t *arg[3];
} node_t;

/**
 * Represents an element of the parse tree which is either a leaf or an inner
 * node.
 */
struct elem {
    /**
     * Union of leaf and inner node.
     */
    union {
        /**
         * The associated leaf if type == LEAF.
         */
        leaf_t leaf;
        /**
         * The associated inner node if type == NODE.
         */
        node_t node;
    } ln;
    /**
     * Type of the element (LEAF or NODE).
     */
    int type;
    /**
     * The file this element comes from.
     */
    char *file;
    /**
     * The package this element comes from.
     */
    char *package;
    /**
     * The number of the line this element is contained in.
     */
    int line;
    /**
     * Pointer to the next element to execute or NULL if this is the last
     * element.
     */
    elem_t *next;
};

#endif
