/*****************************************************************************
 *  @file vartype/regexp_part.h
 *  Functions for parsing and assembling regular expressions associated with
 *  variable types and variable type extensions.
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

#ifndef MKFLI4L_VARTYPE_REGEX_PART_H_
#define MKFLI4L_VARTYPE_REGEX_PART_H_

#include <libmkfli4l/defs.h>
#include <vartype.h>

struct vartype_regex_part_t;

/**
 * Parses a regular expression, returning a linked list of expression parts.
 *
 * @param expr
 *  The regular expression to parse.
 * @return
 *  A linked list of vartype_regex_part_t objects describing the parts of
 *  the regular expression. Currently, these parts are either (sub)strings or
 *  references to variable types.
 */
struct vartype_regex_part_t *
vartype_regex_parse(char const *expr);

/**
 * Destroys an expression part.
 *
 * @param THIS
 *  The expression part to destroy.
 */
void
vartype_regex_free(struct vartype_regex_part_t *THIS);

/**
 * Assembles the regular expression from a list of expression parts.
 *
 * @param THIS
 *  The expression part to process.
 * @param expand
 *  If TRUE, references to variable types are expanded recursively. If a cycle
 *  is detected, this function fails.
 * @return
 *  A dynamically allocated string containing the desired regular expression or
 *  NULL if expansion failed.
 */
char *
vartype_regex_assemble(
    struct vartype_regex_part_t *THIS,
    BOOL expand
);

#endif
