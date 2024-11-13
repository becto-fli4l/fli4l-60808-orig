/*****************************************************************************
 *  @file vartype/core.h
 *  Functions for processing variable types (private interface).
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

#ifndef MKFLI4L_VARTYPE_CORE_H_
#define MKFLI4L_VARTYPE_CORE_H_

#include <libmkfli4l/defs.h>
#include <libmkfli4l/array.h>
#include <container/hashmap.h>
#include <vartype.h>

/**
 * String printed when the name of an anonymous type is needed.
 */
#define VARTYPE_NAME_ANONYMOUS "(anonymous)"

struct vartype_regex_part_t;
struct vartype_compiled_state_t;
struct vartype_extension_t;

/**
 * Represents a variable type represented by a regular expression.
 */
struct vartype_t {
    /**
     * The name of the type. If NULL, the type is anonymous.
     */
    char *name;
    /**
     * The regular expression associated with this type, represented by a
     * linked list of expression parts. If NULL, the variable type is currently
     * referenced by other regular expressions only and the actual definition
     * has not been encountered yet.
     */
    struct vartype_regex_part_t *expr;
    /**
     * The error message associated with this type.
     */
    char *error_msg;
    /**
     * A comment associated with this type.
     */
    char *comment;
    /**
     * The location of the type definition. This member is never NULL.
     */
    struct location_t *location;
    /**
     * The state of compilation. If NULL, the variable type has not been
     * compiled yet.
     */
    struct vartype_compiled_state_t *compiled_state;
    /**
     * TRUE if this variable type is currently expanded, FALSE otherwise. This
     * flag is used to detect cycles in regular expressions.
     */
    BOOL expansion_in_progress;
    /**
     * Type extensions.
     */
    struct vartype_extension_t *extensions;
    /**
     * True if this is a temporary variable type.
     */
    BOOL temporary;
    /**
     * True if this is a predefined variable type. A predefined type may be
     * overridden by another type definition provided the expression is the
     * same.
     */
    BOOL predefined;
    /**
     * True if the regular expression has to be completed by ^( and )$.
     */
    BOOL incomplete;
};

/**
 * Hash map of named variable types indexed by name. Note that anonymous
 * variable types are not put into this hash map.
 */
extern struct hashmap_t *vartypes;
/**
 * Array of unnamed variable types.
 */
extern array_t *anon_vartypes;

/**
 * Initializes the core module.
 */
void
vartype_core_init(void);

/**
 * Finalizes the core module.
 */
void
vartype_core_fini(void);

#endif
