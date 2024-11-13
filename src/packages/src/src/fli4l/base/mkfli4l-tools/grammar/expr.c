/*****************************************************************************
 *  @file grammar/expr.c
 *  Functions for scanning/parsing expressions.
 *
 *  Copyright (c) 2015-2016 The fli4l team
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
#include "grammar/expr.h"

struct expr_parse_tree_t *
expr_parse_tree_create(void)
{
    struct expr_parse_tree_t *tree = (struct expr_parse_tree_t *)
        malloc(sizeof(struct expr_parse_tree_t));
    tree->ok = TRUE;
    tree->expr = NULL;
    return tree;
}

void
expr_parse_tree_destroy(struct expr_parse_tree_t *tree)
{
    if (tree->expr) {
        expr_destroy(tree->expr);
    }
    free(tree);
}
