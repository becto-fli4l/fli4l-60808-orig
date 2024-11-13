/*----------------------------------------------------------------------------
 *  tree.h - functions for creating a parse tree for extended check scripts
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
 *----------------------------------------------------------------------------
 */

#ifndef LIBMKFLI4L_TREE_H_
#define LIBMKFLI4L_TREE_H_

typedef struct elem elem_t;

elem_t *mknode(int op, elem_t *arg1, elem_t *arg2, elem_t *arg3, int line);
elem_t *mkleaf(int type, char const *value);
elem_t *add_node(elem_t *node1, elem_t *node2);

void add_script(elem_t *node);
void walk_tree(void);

/**
 * Reorder scripts such that the "base" scripts are executed first and the
 * "circuits" scripts are executed last.
 */
void reorder_check_scripts(void);

#define NUMEQUAL 1024
#define VEREQUAL 1025
#define SCRIPT   2048
#define SPLIT_NUMERIC    4096

#endif
