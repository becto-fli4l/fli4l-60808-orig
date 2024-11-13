/*****************************************************************************
 *  @file grammar/tests/test_scanner_vardef.c
 *  Functions for testing the scanning of variable definition files.
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
#include "grammar/vardef.h"
#include "grammar/vardef_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.vardef.scanner"

#define TEST_PACKAGE "test"

/**
 * Represents a token recognized by the scanner.
 */
struct vardef_token_t {
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
        = package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, name);
    array_t *result = init_array(32);

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (vardef_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", name);
        }
        else {
            vardef_set_in(f, scanner);

            while (TRUE) {
                struct vardef_token_t *token = (struct vardef_token_t *)
                    safe_malloc(sizeof(struct vardef_token_t));
                token->type
                    = vardef_lex(&token->value, &token->location, scanner);
                append_new_elem(result, token);
                if (token->type == 0) {
                    break;
                }
            }

            vardef_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable definitions file %s\n", name);
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
    ARRAY_ITER(it, array, token, struct vardef_token_t) {
        switch (token->type) {
        case ID :
        case TEMPLATE_ID :
            identifier_destroy(token->value.id);
            break;
        case REGEX :
        case COMMENT :
        case UNKNOWN :
            free(token->value.str);
            break;
        case QSTRING :
            qstr_destroy(token->value.qstr);
            break;
        case NUMBER :
            number_destroy(token->value.number);
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
mu_test_begin(test_vardef_scanner_empty_file)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/empty.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (1).
 */
mu_test_begin(test_vardef_scanner_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (2).
 */
mu_test_begin(test_vardef_scanner_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single definition only.
 */
mu_test_begin(test_vardef_scanner_def1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented definition.
 */
mu_test_begin(test_vardef_scanner_def2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented definition without a
 * default value.
 */
mu_test_begin(test_vardef_scanner_def3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_3.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(36, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(37, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(37, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented definition of an optional
 * variable.
 */
mu_test_begin(test_vardef_scanner_def4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_4.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NUMBER, token->type);
    mu_assert_eq_int(TRUE, token->value.number->valid);
    mu_assert_eq_int(123, token->value.number->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(41, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(42, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(42, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented definition of a really
 * optional array.
 */
mu_test_begin(test_vardef_scanner_def5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_5.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_%_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1_%_XY", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition of a really optional
 * array together with an array bound variable.
 */
mu_test_begin(test_vardef_scanner_def6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_6.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_N", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_%_XY", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_N", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NUMBER, token->type);
    mu_assert_eq_int(TRUE, token->value.number->valid);
    mu_assert_eq_int(123, token->value.number->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1_%_XY", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(33, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(57, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(58, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(58, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing two definitions of which one variable depends
 * on the other one.
 */
mu_test_begin(test_vardef_scanner_def7a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7a.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("YESNO", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR2", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing two definitions of which one variable depends
 * on the inverse of another one.
 */
mu_test_begin(test_vardef_scanner_def7b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7b.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("YESNO", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR2", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using an enabling condition with
 * a logical AND.
 */
mu_test_begin(test_vardef_scanner_def7c)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7c.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_Y", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using an enabling condition with
 * a logical OR.
 */
mu_test_begin(test_vardef_scanner_def7d)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7d.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_Y", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using an enabling condition with
 * an equality expression.
 */
mu_test_begin(test_vardef_scanner_def7e)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7e.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using an enabling condition with
 * an inequality expression.
 */
mu_test_begin(test_vardef_scanner_def7f)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7f.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using a complex enabling
 * condition.
 */
mu_test_begin(test_vardef_scanner_def7g)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7g.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_YZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("def", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(35, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(37, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(41, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(41, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_A", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(42, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(47, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(47, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(49, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(50, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(52, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(52, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("yes|no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(53, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(60, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(61, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(61, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(63, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(63, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(65, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(71, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(73, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(77, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(78, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(78, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a variable using a condition with an invalid
 * quoted string.
 */
mu_test_begin(test_vardef_scanner_def7h)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_7h.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("V", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a definition with an anonymous variable type.
 */
mu_test_begin(test_vardef_scanner_def8)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_8.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(REGEX, token->type);
    mu_assert_eq_str("abc|(def)+|(e*fg*)?", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(59, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(60, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(60, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_vardef_scanner_def_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single definition and comments around it.
 */
mu_test_begin(test_vardef_scanner_def_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a multi-line commented single definition,
 * comments above and below, and LF line endings.
 */
mu_test_begin(test_vardef_scanner_def_comment3a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_comment_3a.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a multi-line commented single definition,
 * comments above and below, and CR/LF line endings.
 */
mu_test_begin(test_vardef_scanner_def_comment3b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_comment_3b.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (1).
 */
mu_test_begin(test_vardef_scanner_string_escape1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("\\abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (2).
 */
mu_test_begin(test_vardef_scanner_string_escape2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("a\\bc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (3).
 */
mu_test_begin(test_vardef_scanner_string_escape3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_3.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (4).
 */
mu_test_begin(test_vardef_scanner_string_escape4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_4.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\\abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (5).
 */
mu_test_begin(test_vardef_scanner_string_escape5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_5.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\\bc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (6).
 */
mu_test_begin(test_vardef_scanner_string_escape6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_6.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (7).
 */
mu_test_begin(test_vardef_scanner_string_escape7)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_string_escape_7.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\\\"abc\"\''\"", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a wrong identifier.
 */
mu_test_begin(test_vardef_scanner_def_wrong1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_wrong_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("VaR1_%_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VaR1_%_XY", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("VAr1_%_XY", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAr1_%_XY", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("var1_%_xy", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of var1_%_xy", token->value.str);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a wrong condition.
 */
mu_test_begin(test_vardef_scanner_def_wrong2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_wrong_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_%_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str(":", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1_%_XY", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an unclosed string.
 */
mu_test_begin(test_vardef_scanner_def_wrong3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_def_wrong_3.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_%_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123 # definition of VAR1_%_XY ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition with a BOM.
 */
mu_test_begin(test_vardef_scanner_bom)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_bom.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition with TABs.
 */
mu_test_begin(test_vardef_scanner_tabs)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tabs.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("a\tbc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of\tVAR1", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(59, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(60, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(60, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single template definition only.
 */
mu_test_begin(test_vardef_scanner_tmpldef1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_1.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented template definition.
 */
mu_test_begin(test_vardef_scanner_tmpldef2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_2.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR.+", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(42, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(43, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(43, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented template definition
 * without a default value.
 */
mu_test_begin(test_vardef_scanner_tmpldef3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_3.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR.+", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented definition of an optional
 * variable template.
 */
mu_test_begin(test_vardef_scanner_tmpldef4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_4.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NUMBER, token->type);
    mu_assert_eq_int(TRUE, token->value.number->valid);
    mu_assert_eq_int(123, token->value.number->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR.+", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(44, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(45, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(45, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single commented template definition of a
 * really optional array.
 */
mu_test_begin(test_vardef_scanner_tmpldef5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_5.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR1_%_.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(15, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1_%_.+", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(56, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(56, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented template definition of a really
 * optional array together with an array bound variable.
 */
mu_test_begin(test_vardef_scanner_tmpldef6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_6.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_N", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OPT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR1_%_.+", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1_N", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NUMBER, token->type);
    mu_assert_eq_int(TRUE, token->value.number->valid);
    mu_assert_eq_int(123, token->value.number->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR1_%_.+", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(58, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(59, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(59, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition which depends on a
 * variable.
 */
mu_test_begin(test_vardef_scanner_tmpldef7a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7a.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("YESNO", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition which depends on the
 * inverse of a variable.
 */
mu_test_begin(test_vardef_scanner_tmpldef7b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7b.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("YESNO", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("VAR1", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using an enabling
 * condition with a logical AND.
 */
mu_test_begin(test_vardef_scanner_tmpldef7c)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7c.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_Y", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(35, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using an enabling
 * condition with a logical OR.
 */
mu_test_begin(test_vardef_scanner_tmpldef7d)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7d.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_Y", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(35, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using an enabling
 * condition with an equality expression.
 */
mu_test_begin(test_vardef_scanner_tmpldef7e)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7e.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(35, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using an enabling
 * condition with an inequality expression.
 */
mu_test_begin(test_vardef_scanner_tmpldef7f)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7f.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(35, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using a complex
 * enabling condition.
 */
mu_test_begin(test_vardef_scanner_tmpldef7g)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7g.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_YZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("def", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(41, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(43, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(43, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(44, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(44, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_A", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(45, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(49, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(50, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(50, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(52, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(55, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(55, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("yes|no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(56, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(63, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(64, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(64, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(66, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(66, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(68, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(74, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(76, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(80, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(81, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(81, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition using a condition with
 * an invalid quoted string.
 */
mu_test_begin(test_vardef_scanner_tmpldef7h)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_7h.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%V.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_X", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("123", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a template definition with an anonymous
 * variable type.
 */
mu_test_begin(test_vardef_scanner_tmpldef8)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vardef_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vardef_tmpldef_8.txt");
    init_array_iterator(&it, result);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(TEMPLATE_ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("%VAR.+", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NONE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(REGEX, token->type);
    mu_assert_eq_str("abc|(def)+|(e*fg*)?", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(40, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of VAR.+", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(42, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(62, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NEWLINE, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(63, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(63, token->location.last_column);

    token = (struct vardef_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

void
test_vardef_scanner(void)
{
    mu_run_test(test_vardef_scanner_empty_file);
    mu_run_test(test_vardef_scanner_comment1);
    mu_run_test(test_vardef_scanner_comment2);
    mu_run_test(test_vardef_scanner_def1);
    mu_run_test(test_vardef_scanner_def2);
    mu_run_test(test_vardef_scanner_def3);
    mu_run_test(test_vardef_scanner_def4);
    mu_run_test(test_vardef_scanner_def5);
    mu_run_test(test_vardef_scanner_def6);
    mu_run_test(test_vardef_scanner_def7a);
    mu_run_test(test_vardef_scanner_def7b);
    mu_run_test(test_vardef_scanner_def7c);
    mu_run_test(test_vardef_scanner_def7d);
    mu_run_test(test_vardef_scanner_def7e);
    mu_run_test(test_vardef_scanner_def7f);
    mu_run_test(test_vardef_scanner_def7g);
    mu_run_test(test_vardef_scanner_def7h);
    mu_run_test(test_vardef_scanner_def8);
    mu_run_test(test_vardef_scanner_def_comment1);
    mu_run_test(test_vardef_scanner_def_comment2);
    mu_run_test(test_vardef_scanner_def_comment3a);
    mu_run_test(test_vardef_scanner_def_comment3b);
    mu_run_test(test_vardef_scanner_string_escape1);
    mu_run_test(test_vardef_scanner_string_escape2);
    mu_run_test(test_vardef_scanner_string_escape3);
    mu_run_test(test_vardef_scanner_string_escape4);
    mu_run_test(test_vardef_scanner_string_escape5);
    mu_run_test(test_vardef_scanner_string_escape6);
    mu_run_test(test_vardef_scanner_string_escape7);
    mu_run_test(test_vardef_scanner_def_wrong1);
    mu_run_test(test_vardef_scanner_def_wrong2);
    mu_run_test(test_vardef_scanner_def_wrong3);
    mu_run_test(test_vardef_scanner_bom);
    mu_run_test(test_vardef_scanner_tabs);
    mu_run_test(test_vardef_scanner_tmpldef1);
    mu_run_test(test_vardef_scanner_tmpldef2);
    mu_run_test(test_vardef_scanner_tmpldef3);
    mu_run_test(test_vardef_scanner_tmpldef4);
    mu_run_test(test_vardef_scanner_tmpldef5);
    mu_run_test(test_vardef_scanner_tmpldef6);
    mu_run_test(test_vardef_scanner_tmpldef7a);
    mu_run_test(test_vardef_scanner_tmpldef7b);
    mu_run_test(test_vardef_scanner_tmpldef7c);
    mu_run_test(test_vardef_scanner_tmpldef7d);
    mu_run_test(test_vardef_scanner_tmpldef7e);
    mu_run_test(test_vardef_scanner_tmpldef7f);
    mu_run_test(test_vardef_scanner_tmpldef7g);
    mu_run_test(test_vardef_scanner_tmpldef7h);
    mu_run_test(test_vardef_scanner_tmpldef8);
}
