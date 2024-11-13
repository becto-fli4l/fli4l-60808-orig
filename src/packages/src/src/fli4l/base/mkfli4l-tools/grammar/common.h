/*****************************************************************************
 *  @file grammar/common.h
 *  Generic functions for scanning/parsing.
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

#ifndef MKFLI4L_GRAMMAR_COMMON_H_
#define MKFLI4L_GRAMMAR_COMMON_H_

#include "libmkfli4l/defs.h"
#include <expression/expr.h>

/**
 * Represents an identifier.
 */
struct identifier_t {
    /**
     * The name of the identifier.
     */
    char *name;
    /**
     * TRUE if name is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates an identifier_t object.
 *
 * @param name
 *  The name of the identifier.
 * @param valid
 *  TRUE if identifier is valid, FALSE otherwise.
 * @return
 *  An identifier_t object. Use identifier_destroy() to free it after use.
 */
struct identifier_t *
identifier_create(char const *name, BOOL valid);

/**
 * Destroys an identifier_t object. May be NULL.
 *
 * @param identifier
 *  The object to destroy.
 */
void
identifier_destroy(struct identifier_t *identifier);

/**
 * Represents a quoted string.
 */
struct qstr_t {
    /**
     * The delimiter used for quoting.
     */
    char delim;
    /**
     * The string value without quotes.
     */
    char *value;
    /**
     * TRUE if string is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a qstr_t object.
 *
 * @param delim
 *  The delimiter used for quoting.
 * @param value
 *  The string value without quotes. Before being stored, this string is
 *  processed in a special way. At first, all line breaks are canonicalized to
 *  LF only by removing all CRs. Then, if "deescape" is true, escape characters
 *  are removed; if an escape character is found before a line break, the line
 *  break is also removed. At last, all remaining line breaks and directly
 *  following whitespace (space, horizontal and vertical TAB) are replaced by a
 *  single space character.
 * @param deescape
 *  If TRUE, escape characters are removed from the string.
 * @param valid
 *  TRUE if string is valid, FALSE otherwise.
 * @return
 *  A qstr_t object. Use qstr_destroy() to free it after use.
 */
struct qstr_t *
qstr_create(char delim, char const *value, BOOL deescape, BOOL valid);

/**
 * Destroys a qstr_t object. May be NULL.
 *
 * @param qstr
 *  The object to destroy.
 */
void
qstr_destroy(struct qstr_t *qstr);

/**
 * Creates a comment from a raw comment string. The process strips leading
 * '#' characters at the beginning of each line as well as following white
 * space.
 *
 * @param value
 *  The raw comment value.
 * @return
 *  The processed comment value.
 */
char *
comment_create(char const *value);

/**
 * Represents a number.
 */
struct number_t {
    /**
     * The value of the number.
     */
    unsigned value;
    /**
     * TRUE if integer is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates a number_t object.
 *
 * @param value
 *  The value of the number.
 * @param valid
 *  TRUE if number is valid, FALSE otherwise.
 * @return
 *  An number_t object. Use number_destroy() to free it after use.
 */
struct number_t *
number_create(unsigned value, BOOL valid);

/**
 * Destroys a number_t object. May be NULL.
 *
 * @param number
 *  The object to destroy.
 */
void
number_destroy(struct number_t *number);

/**
 * Represents an expression.
 */
struct expression_t {
    /**
     * The encapsulated expression.
     */
    struct expr_t *expr;
    /**
     * TRUE if expression is valid, FALSE otherwise.
     */
    BOOL valid;
};

/**
 * Creates an expression_t object.
 *
 * @param expr
 *  The encapsulated expression.
 * @param valid
 *  TRUE if expression is valid, FALSE otherwise.
 * @return
 *  An expression_t object. Use expression_destroy() to free it after use.
 */
struct expression_t *
expression_create(struct expr_t *expr, BOOL valid);

/**
 * Destroys an expression_t object.
 *
 * @param expression
 *  The object to destroy. May be NULL.
 * @param destroy_expr
 *  If TRUE, the encapsulated expression is destroyed. Use FALSE if the
 *  expression has been moved elsewhere without being cloned.
 */
void
expression_destroy(struct expression_t *expression, BOOL destroy_expr);

#endif
