/*****************************************************************************
 *  @file grammar/vardef.c
 *  Functions for scanning/parsing files containing variable definitions.
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
#include "grammar/vardef.h"

struct vardef_typeref_t *
vardef_typeref_create_named(struct identifier_t *vartype)
{
    struct vardef_typeref_t *typeref = (struct vardef_typeref_t *)
        malloc(sizeof(struct vardef_typeref_t));
    typeref->u.id = vartype;
    typeref->named = TRUE;
    typeref->valid = vartype->valid;
    return typeref;
}

struct vardef_typeref_t *
vardef_typeref_create_unnamed(char *vartype)
{
    struct vardef_typeref_t *typeref = (struct vardef_typeref_t *)
        malloc(sizeof(struct vardef_typeref_t));
    typeref->u.regex = vartype;
    typeref->named = FALSE;
    typeref->valid = TRUE;
    return typeref;
}

void
vardef_typeref_destroy(struct vardef_typeref_t *typeref)
{
    if (typeref) {
        if (typeref->named) {
            identifier_destroy(typeref->u.id);
        }
        else {
            free(typeref->u.regex);
        }
        free(typeref);
    }
}

struct vardef_t *
vardef_create(
    BOOL optional,
    struct identifier_t *id,
    struct expression_t *cond,
    struct identifier_t *array_bound,
    struct vardef_typeref_t *type,
    struct expression_t *defvalue,
    char *comment,
    struct location_t *location
)
{
    struct vardef_t *def = (struct vardef_t *) malloc(sizeof(struct vardef_t));
    def->optional = optional;
    def->id = id;
    def->cond = cond;
    def->array_bound = array_bound;
    def->type = type;
    def->defvalue = defvalue;
    def->comment = comment;
    def->location = location;
    def->valid = id->valid && type->valid;
    if (cond) {
        def->valid = def->valid && cond->valid;
    }
    if (array_bound) {
        def->valid = def->valid && array_bound->valid;
    }
    if (defvalue) {
        def->valid = def->valid && defvalue->valid;
    }
    return def;
}

void
vardef_destroy(struct vardef_t *def)
{
    if (def) {
        identifier_destroy(def->id);
        expression_destroy(def->cond, TRUE);
        identifier_destroy(def->array_bound);
        vardef_typeref_destroy(def->type);
        expression_destroy(def->defvalue, TRUE);
        free(def->comment);
        location_destroy(def->location);
        free(def);
    }
}

struct vardef_template_t *
vardef_create_template(
    BOOL optional,
    struct identifier_t *id,
    struct expression_t *cond,
    struct identifier_t *array_bound,
    struct vardef_typeref_t *type,
    struct expression_t *defvalue,
    char *comment,
    struct location_t *location
)
{
    struct vardef_template_t *def
        = (struct vardef_template_t *) malloc(sizeof(struct vardef_template_t));
    def->optional = optional;
    def->id = id;
    def->cond = cond;
    def->array_bound = array_bound;
    def->type = type;
    def->defvalue = defvalue;
    def->comment = comment;
    def->location = location;
    def->valid = id->valid && type->valid;
    if (cond) {
        def->valid = def->valid && cond->valid;
    }
    if (array_bound) {
        def->valid = def->valid && array_bound->valid;
    }
    if (defvalue) {
        def->valid = def->valid && defvalue->valid;
    }
    return def;
}

void
vardef_destroy_template(struct vardef_template_t *def)
{
    if (def) {
        identifier_destroy(def->id);
        expression_destroy(def->cond, TRUE);
        identifier_destroy(def->array_bound);
        vardef_typeref_destroy(def->type);
        expression_destroy(def->defvalue, TRUE);
        free(def->comment);
        location_destroy(def->location);
        free(def);
    }
}

struct vardef_parse_tree_t *
vardef_parse_tree_create(void)
{
    struct vardef_parse_tree_t *tree = (struct vardef_parse_tree_t *)
        malloc(sizeof(struct vardef_parse_tree_t));
    tree->ok = TRUE;
    tree->definitions = init_array(1024);
    tree->template_definitions = init_array(16);
    return tree;
}

void
vardef_parse_tree_add_definition(
    struct vardef_parse_tree_t *tree,
    struct vardef_t *def
)
{
    append_new_elem(tree->definitions, def);
    if (!def->valid) {
        tree->ok = FALSE;
    }
}

void
vardef_parse_tree_add_template_definition(
    struct vardef_parse_tree_t *tree,
    struct vardef_template_t *def
)
{
    append_new_elem(tree->template_definitions, def);
    if (!def->valid) {
        tree->ok = FALSE;
    }
}

void
vardef_parse_tree_destroy(struct vardef_parse_tree_t *tree)
{
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpl_def, struct vardef_template_t);

    ARRAY_ITER_LOOP(it, tree->definitions, def) {
        vardef_destroy(def);
    }
    ARRAY_ITER_LOOP(it2, tree->template_definitions, tmpl_def) {
        vardef_destroy_template(tmpl_def);
    }
    free_array(tree->definitions);
    free_array(tree->template_definitions);
    free(tree);
}
