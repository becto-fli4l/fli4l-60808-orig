/*----------------------------------------------------------------------------
 *  tree_debug.h - debugging parse trees for extended check scripts
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

#ifndef LIBMKFLI4L_TREE_DEBUG_H_
#define LIBMKFLI4L_TREE_DEBUG_H_

#define NONE 0
#define SOMETHING 3
#define ANYTHING 4
#define OPT 8

void expect_types(elem_t *p, int arg1, int arg2, int arg3, int line);
void expect_node(elem_t *p, int line, int arg, int opt);

void dump_elem(int log_level, elem_t *p);
char const *get_op_name(int op);

#endif
