/*****************************************************************************
 *  @file grammar/common.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libmkfli4l/defs.h"
#include "libmkfli4l/str.h"
#include "grammar/common.h"

struct identifier_t *
identifier_create(char const *name, BOOL valid)
{
    struct identifier_t *identifier = (struct identifier_t *)
        malloc(sizeof(struct identifier_t));
    identifier->name = strdup(name);
    identifier->valid = valid;
    return identifier;
}

void
identifier_destroy(struct identifier_t *identifier)
{
    if (identifier) {
        free(identifier->name);
        free(identifier);
    }
}

/**
 * Changes all line breaks to LF by removing all CRs found in the string.
 *
 * @param s
 *  The string.
 */
static void
canonicalize_line_endings(char *s)
{
    char *tgt = s;

    while (*s) {
        if (*s != '\r') {
            *tgt++ = *s;
        }
        ++s;
    }
    *tgt = '\0';
}

/**
 * Removes one level of escape characters (\\) from the string. If such an
 * escape character is found before a line break (LF), the line break is also
 * removed.
 *
 * @param s
 *  The string.
 */
static void
remove_escape_characters(char *s)
{
    char *tgt = s;
    BOOL escape_active = FALSE;

    while (*s) {
        if (*s == '\\' && !escape_active) {
            escape_active = TRUE;
        }
        else {
            if (*s != '\n' || !escape_active) {
                *tgt++ = *s;
            }
            escape_active = FALSE;
        }
        ++s;
    }
    *tgt = '\0';
}

/**
 * Concatenates all lines by replacing all LFs and directly following
 * whitespace (space, horizontal and vertical TAB, LF) by a single space
 * character.
 *
 * @param s
 *  The string.
 * @param comment_mode
 *  If TRUE, the contents of a comment are to be processed. This strips leading
 * '#' characters at the beginning of each line as well as following white
 * space.
 */
static void
concatenate_lines(char *s, BOOL comment_mode)
{
    char *tgt = s;
    BOOL eat_ws = comment_mode;

    while (*s) {
        if (*s == '\n') {
            *tgt++ = ' ';
            ++s;
            eat_ws = TRUE;
        }
        if (eat_ws) {
            while (*s && (*s == ' ' || *s == '\t' || *s == '\v' || *s == '\n')) {
                ++s;
            }
            if (*s && comment_mode) {
                ++s; /* skip '#' */
                while (*s && (*s == ' ' || *s == '\t' || *s == '\v' || *s == '\n')) {
                    ++s;
                }
            }
            eat_ws = FALSE;
        }
        else {
            *tgt++ = *s++;
        }
    }
    *tgt = '\0';
}

struct qstr_t *
qstr_create(char delim, char const *value, BOOL deescape, BOOL valid)
{
    struct qstr_t *qstr = (struct qstr_t *) malloc(sizeof(struct qstr_t));
    qstr->delim = delim;
    qstr->value = strdup(value);
    qstr->valid = valid;
    canonicalize_line_endings(qstr->value);
    if (deescape) {
        remove_escape_characters(qstr->value);
    }
    concatenate_lines(qstr->value, FALSE);
    return qstr;
}

void
qstr_destroy(struct qstr_t *qstr)
{
    if (qstr) {
        free(qstr->value);
        free(qstr);
    }
}

char *
comment_create(char const *value)
{
    char *result = strdup(value);
    canonicalize_line_endings(result);
    concatenate_lines(result, TRUE);
    return result;
}

struct number_t *
number_create(unsigned value, BOOL valid)
{
    struct number_t *number = (struct number_t *)
        malloc(sizeof(struct number_t));
    number->value = value;
    number->valid = valid;
    return number;
}

void
number_destroy(struct number_t *number)
{
    if (number) {
        free(number);
    }
}

struct expression_t *
expression_create(struct expr_t *expr, BOOL valid)
{
    struct expression_t *expression = (struct expression_t *)
        malloc(sizeof(struct expression_t));
    expression->expr = expr;
    expression->valid = valid;
    return expression;
}

void
expression_destroy(struct expression_t *expression, BOOL destroy_expr)
{
    if (expression) {
        if (destroy_expr) {
            expr_destroy(expression->expr);
        }
        free(expression);
    }
}
