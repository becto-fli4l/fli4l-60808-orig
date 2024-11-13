/*****************************************************************************
 *  @file grammar/vartype.c
 *  Functions for scanning/parsing files containing variable types.
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
#include "grammar/common.h"
#include "grammar/vartype.h"

struct vartype_def_t *
vartype_def_create(
    struct identifier_t *id,
    struct qstr_t *regex,
    struct qstr_t *errmsg,
    char *comment,
    struct location_t *location
)
{
    struct vartype_def_t *def = (struct vartype_def_t *)
        malloc(sizeof(struct vartype_def_t));
    def->id = id;
    def->regex = regex;
    def->errmsg = errmsg;
    def->comment = comment;
    def->location = location;
    def->valid = id->valid && regex->valid && errmsg->valid;
    return def;
}

void
vartype_def_destroy(struct vartype_def_t *def)
{
    if (def) {
        identifier_destroy(def->id);
        qstr_destroy(def->regex);
        qstr_destroy(def->errmsg);
        free(def->comment);
        location_destroy(def->location);
        free(def);
    }
}

struct vartype_ext_t *
vartype_ext_create(
    struct identifier_t *id,
    struct expression_t *cond,
    struct qstr_t *regex,
    struct qstr_t *errmsg,
    char *comment,
    struct location_t *location
)
{
    struct vartype_ext_t *ext = (struct vartype_ext_t *)
        malloc(sizeof(struct vartype_ext_t));
    ext->id = id;
    ext->cond = cond;
    ext->regex = regex;
    ext->errmsg = errmsg;
    ext->comment = comment;
    ext->location = location;
    ext->valid = id->valid && regex->valid && errmsg->valid;
    if (cond) {
        ext->valid = ext->valid && cond->valid;
    }
    return ext;
}

void
vartype_ext_destroy(struct vartype_ext_t *ext)
{
    if (ext) {
        identifier_destroy(ext->id);
        expression_destroy(ext->cond, TRUE);
        qstr_destroy(ext->regex);
        qstr_destroy(ext->errmsg);
        free(ext->comment);
        location_destroy(ext->location);
        free(ext);
    }
}

struct vartype_parse_tree_t *
vartype_parse_tree_create(void)
{
    struct vartype_parse_tree_t *tree = (struct vartype_parse_tree_t *)
        malloc(sizeof(struct vartype_parse_tree_t));
    tree->ok = TRUE;
    tree->definitions = init_array(1024);
    tree->extensions = init_array(128);
    return tree;
}

void
vartype_parse_tree_add_definition(
    struct vartype_parse_tree_t *tree,
    struct vartype_def_t *def
)
{
    append_new_elem(tree->definitions, def);
    if (!def->valid) {
        tree->ok = FALSE;
    }
}

void
vartype_parse_tree_add_extension(
    struct vartype_parse_tree_t *tree,
    struct vartype_ext_t *ext
)
{
    append_new_elem(tree->extensions, ext);
    if (!ext->valid) {
        tree->ok = FALSE;
    }
}

void
vartype_parse_tree_destroy(struct vartype_parse_tree_t *tree)
{
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    ARRAY_ITER_LOOP(it_def, tree->definitions, def) {
        vartype_def_destroy(def);
    }
    ARRAY_ITER_LOOP(it_ext, tree->extensions, ext) {
        vartype_ext_destroy(ext);
    }
    free_array(tree->definitions);
    free_array(tree->extensions);
    free(tree);
}
