/*****************************************************************************
 *  @file grammar/tests/test_scanner_varass.c
 *  Functions for testing the scanning of variable assignment files.
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

#include <testing/tests.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/array.h>
#include <stdlib.h>
#include <string.h>
#include "grammar/varass.h"
#include "grammar/varass_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.varass.scanner"

#define TEST_PACKAGE "test"

/**
 * Represents a token recognized by the scanner.
 */
struct varass_token_t {
    /**
     * The token's type.
     */
    int type;
    /**
     * The token's value.
     */
    YYSTYPE value;
    /**
     * The token's location.
     */
    YYLTYPE location;
};

/**
 * Runs the scanner for a given input file.
 *
 * @param name
 *  The name of the input file.
 * @return
 *  An array of tokens which have been recognized.
 */
static array_t *
run_scanner(char const *name)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, name);
    array_t *result = init_array(32);

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (varass_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", name);
        }
        else {
            varass_set_in(f, scanner);

            while (TRUE) {
                struct varass_token_t *token = (struct varass_token_t *)
                    safe_malloc(sizeof(struct varass_token_t));
                token->type
                    = varass_lex(&token->value, &token->location, scanner);
                append_new_elem(result, token);
                if (token->type == 0) {
                    break;
                }
            }

            varass_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable assignment file %s\n", name);
    }

    return result;
}

/**
 * Frees the tokens in an array.
 *
 * @param array
 *  The token array.
 */
static void
free_tokens(array_t *array)
{
    ARRAY_ITER(it, array, token, struct varass_token_t) {
        switch (token->type) {
        case ID :
            identifier_destroy(token->value.id);
            break;
        case COMMENT :
        case UNKNOWN :
            free(token->value.str);
            break;
        case QSTRING :
            qstr_destroy(token->value.qstr);
            break;
        default :
            break;
        }
        free(token);
    }
    free_array(array);
}

/**
 * Tests scanning empty file.
 */
mu_test_begin(test_varass_scanner_empty_file)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/empty.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (1).
 */
mu_test_begin(test_varass_scanner_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (2).
 */
mu_test_begin(test_varass_scanner_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing uncommented assignments.
 */
mu_test_begin(test_varass_scanner_ass1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_1.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR2", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"'", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented assignment.
 */
mu_test_begin(test_varass_scanner_ass2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_2.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("setting VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented assignment distributed across
 * many lines.
 */
mu_test_begin(test_varass_scanner_ass3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_3.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("setting VAR1", token->value.str);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_varass_scanner_ass_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("setting VAR1", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single definition and comments around it.
 */
mu_test_begin(test_varass_scanner_ass_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing assignments to structure members (new syntax).
 */
mu_test_begin(test_varass_scanner_ass_new_syntax1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_new_syntax1.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("A", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("B", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("x", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("A", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("B", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("C", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("y", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing assignments to array slots (new syntax).
 */
mu_test_begin(test_varass_scanner_ass_new_syntax2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_new_syntax2.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ABSOLUTE_INDEX, token->type);
    mu_assert_eq_int(1, token->value.number);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ABSOLUTE_INDEX, token->type);
    mu_assert_eq_int(2, token->value.number);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("XY", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ABSOLUTE_INDEX, token->type);
    mu_assert_eq_int(33, token->value.number);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"'", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("X", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ABSOLUTE_INDEX, token->type);
    mu_assert_eq_int(3, token->value.number);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(DOT, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("Y", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("z", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing structured assignments (1).
 */
mu_test_begin(test_varass_scanner_ass_structured1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_structured1.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("IP_NET", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("192.168.1.0/24", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("my LAN", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("DEV", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("eth0", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NAME", token->value.id->name);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("LAN", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("COMMENT", token->value.id->name);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("bla", token->value.qstr->value);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing structured assignments (2).
 */
mu_test_begin(test_varass_scanner_ass_structured2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_structured2.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("CIRC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("my WAN", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NAME", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("DSL", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("TYPE", token->value.id->name);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("ppp", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("ENABLED", token->value.id->name);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("yes", token->value.qstr->value);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("UP", token->value.id->name);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("yes", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NETS_IPV4", token->value.id->name);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("0.0.0.0/0", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("default route", token->value.str);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(48, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("CLASS", token->value.id->name);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("Internet", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("PPP", token->value.id->name);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("TYPE", token->value.id->name);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("ethernet", token->value.qstr->value);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("PPPoE", token->value.str);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("USERID", token->value.id->name);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("anonymer", token->value.qstr->value);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("PASSWORD", token->value.id->name);
    mu_assert_eq_int(12, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(12, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(12, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(12, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("surfer", token->value.qstr->value);
    mu_assert_eq_int(12, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(12, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("ETHERNET", token->value.id->name);
    mu_assert_eq_int(13, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(13, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("PPPoE specific settings", token->value.str);
    mu_assert_eq_int(13, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(13, token->location.last_line);
    mu_assert_eq_int(49, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(14, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(14, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("DEV", token->value.id->name);
    mu_assert_eq_int(15, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(15, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(15, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(15, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("eth1", token->value.qstr->value);
    mu_assert_eq_int(15, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(15, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("TYPE", token->value.id->name);
    mu_assert_eq_int(16, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(16, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(16, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(16, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("kernel", token->value.qstr->value);
    mu_assert_eq_int(16, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(16, token->location.last_line);
    mu_assert_eq_int(36, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("more performant", token->value.str);
    mu_assert_eq_int(16, token->location.first_line);
    mu_assert_eq_int(38, token->location.first_column);
    mu_assert_eq_int(16, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(17, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(17, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(18, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(18, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(19, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(19, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing structured assignments (3).
 */
mu_test_begin(test_varass_scanner_ass_structured3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_structured3.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("MULTI", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("first row", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("1", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("2", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("MULTI", token->value.id->name);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("second row", token->value.str);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("3", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("4", token->value.qstr->value);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing structured assignments (4).
 */
mu_test_begin(test_varass_scanner_ass_structured4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_ass_structured4.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("MULTI", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("first row", token->value.str);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("1", token->value.qstr->value);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("2", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("second row", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACE, token->type);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("3", token->value.qstr->value);
    mu_assert_eq_int(10, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(10, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("4", token->value.qstr->value);
    mu_assert_eq_int(11, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(11, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(12, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(12, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACE, token->type);
    mu_assert_eq_int(13, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(13, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing assignments with a BOM.
 */
mu_test_begin(test_varass_scanner_bom)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct varass_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/varass_bom.txt");
    init_array_iterator(&it, result);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR2", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\\"'", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct varass_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

void
test_varass_scanner(void)
{
    mu_run_test(test_varass_scanner_empty_file);
    mu_run_test(test_varass_scanner_comment1);
    mu_run_test(test_varass_scanner_comment2);
    mu_run_test(test_varass_scanner_ass1);
    mu_run_test(test_varass_scanner_ass2);
    mu_run_test(test_varass_scanner_ass3);
    mu_run_test(test_varass_scanner_ass_comment1);
    mu_run_test(test_varass_scanner_ass_comment2);
    mu_run_test(test_varass_scanner_ass_new_syntax1);
    mu_run_test(test_varass_scanner_ass_new_syntax2);
    mu_run_test(test_varass_scanner_ass_structured1);
    mu_run_test(test_varass_scanner_ass_structured2);
    mu_run_test(test_varass_scanner_ass_structured3);
    mu_run_test(test_varass_scanner_ass_structured4);
    mu_run_test(test_varass_scanner_bom);
}
