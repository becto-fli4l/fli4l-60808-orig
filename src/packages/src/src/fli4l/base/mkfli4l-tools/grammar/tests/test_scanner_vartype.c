/*****************************************************************************
 *  @file grammar/tests/test_scanner_vartype.c
 *  Functions for testing the scanning of variable type files.
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
#include "grammar/vartype.h"
#include "grammar/vartype_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.vartype.scanner"

#define TEST_PACKAGE "test"

/**
 * Represents a token recognized by the scanner.
 */
struct vartype_token_t {
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
        = package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, name);
    array_t *result = init_array(32);

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (vartype_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", name);
        }
        else {
            vartype_set_in(f, scanner);

            while (TRUE) {
                struct vartype_token_t *token = (struct vartype_token_t *)
                    safe_malloc(sizeof(struct vartype_token_t));
                token->type
                    = vartype_lex(&token->value, &token->location, scanner);
                append_new_elem(result, token);
                if (token->type == 0) {
                    break;
                }
            }

            vartype_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable types file %s\n", name);
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
    ARRAY_ITER(it, array, token, struct vartype_token_t) {
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
mu_test_begin(test_vartype_scanner_empty_file)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/empty.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (1).
 */
mu_test_begin(test_vartype_scanner_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing only comments (2).
 */
mu_test_begin(test_vartype_scanner_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single definition only.
 */
mu_test_begin(test_vartype_scanner_def1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition (1).
 */
mu_test_begin(test_vartype_scanner_def2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition (2).
 */
mu_test_begin(test_vartype_scanner_def3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_3.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("TYPE123_XYZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(19, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(41, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(60, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines.
 */
mu_test_begin(test_vartype_scanner_def4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_4.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(5, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(43, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid identifier (1).
 */
mu_test_begin(test_vartype_scanner_def_invalid_id1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_id_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("NoNE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid identifier (2).
 */
mu_test_begin(test_vartype_scanner_def_invalid_id2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_id_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("NON%", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid definition due to missing '='.
 */
mu_test_begin(test_vartype_scanner_def_invalid_eq)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_eq.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str("?", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid definition due to an open-ended
 * regular expression.
 */
mu_test_begin(test_vartype_scanner_def_invalid_regex)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_regex.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".* ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid definition due to missing ':'.
 */
mu_test_begin(test_vartype_scanner_def_invalid_colon)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_colon.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str(";", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid definition due to an open-ended
 * error message.
 */
mu_test_begin(test_vartype_scanner_def_invalid_errmsg)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_invalid_errmsg.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_vartype_scanner_def_comment1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_comment_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a single definition and comments around it.
 */
mu_test_begin(test_vartype_scanner_def_comment2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_comment_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a multi-line commented single definition,
 * comments above and below, and LF line endings.
 */
mu_test_begin(test_vartype_scanner_def_comment3a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_comment_3a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a multi-line commented single definition,
 * comments above and below, and CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_def_comment3b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_def_comment_3b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted multi-line strings, and LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline1a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_1a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted multi-line strings, and CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline1b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_1b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted continued multi-line strings, and LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline2a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_2a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(".   *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected     anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted continued multi-line strings, and CR/LF line
 * endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline2b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_2b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(".   *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected     anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted multi-line strings, and LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline3a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_3a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted multi-line strings, and CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline3b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_3b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted "continued" multi-line strings, and LF line
 * endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline4a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_4a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".\\ *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected\\ anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted "continued" multi-line strings, and CR/LF line
 * endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline4b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_4b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".\\ *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected\\ anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline5a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_5a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline5b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_5b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline6a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_6a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline6b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_6b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(7, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(8, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted multi-line strings containing empty lines, and LF
 * line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline7a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_7a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, double-quoted multi-line strings containing empty lines, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline7b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_7b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted multi-line strings containing empty lines, and LF
 * line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline8a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_8a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, single-quoted multi-line strings containing empty lines, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_scanner_string_multiline8b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_8b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(34, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string with
 * empty lines, and LF line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline9a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_9a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string with
 * empty lines, and CR/LF line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline9b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_9b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string with
 * empty lines, and LF line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline10a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_10a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string with
 * empty lines, and CR/LF line endings
 */
mu_test_begin(test_vartype_scanner_string_multiline10b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_multiline_10b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(". *", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(5, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(3, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected ", token->value.qstr->value);
    mu_assert_eq_int(7, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(8, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("anything", token->value.id->name);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(" # definition of NONE ", token->value.qstr->value);
    mu_assert_eq_int(9, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(9, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (1).
 */
mu_test_begin(test_vartype_scanner_string_open1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (2).
 */
mu_test_begin(test_vartype_scanner_string_open2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (3).
 */
mu_test_begin(test_vartype_scanner_string_open3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_3.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\" ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (4).
 */
mu_test_begin(test_vartype_scanner_string_open4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_4.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (5).
 */
mu_test_begin(test_vartype_scanner_string_open5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_5.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (6).
 */
mu_test_begin(test_vartype_scanner_string_open6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_6.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an incomplete string (7).
 */
mu_test_begin(test_vartype_scanner_string_open7)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_open_7.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything ", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (1).
 */
mu_test_begin(test_vartype_scanner_string_escape1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("\\ab", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("\\ab expected", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (2).
 */
mu_test_begin(test_vartype_scanner_string_escape2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("a\\b", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("a\\b expected", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (3).
 */
mu_test_begin(test_vartype_scanner_string_escape3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_3.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("a\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected a\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (4).
 */
mu_test_begin(test_vartype_scanner_string_escape4)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_4.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\\ab", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\\ab expected", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (5).
 */
mu_test_begin(test_vartype_scanner_string_escape5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_5.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\\b", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\\b expected", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (6).
 */
mu_test_begin(test_vartype_scanner_string_escape6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_6.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing strings with escape characters (7).
 */
mu_test_begin(test_vartype_scanner_string_escape7)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_string_escape_7.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("\\\"abc\"''\"", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(25, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(27, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition with a BOM.
 */
mu_test_begin(test_vartype_scanner_bom)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_bom.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented single definition with TABs.
 */
mu_test_begin(test_vartype_scanner_tabs)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_tabs.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(17, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected\tanything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(48, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition \t\tof NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(50, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(78, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a simple extension only.
 */
mu_test_begin(test_vartype_scanner_ext1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a simple commented extension.
 */
mu_test_begin(test_vartype_scanner_ext2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a simple
 * condition.
 */
mu_test_begin(test_vartype_scanner_ext3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_3.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(41, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(43, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(61, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing a regular expression.
 */
mu_test_begin(test_vartype_scanner_ext4a)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_4a.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc\\", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(49, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(51, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(69, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing a regular expression with escaped characters.
 */
mu_test_begin(test_vartype_scanner_ext4b)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_4b.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\"b'\\\\c\"d", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(33, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(36, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(38, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(58, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(60, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(78, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing a logical NOT.
 */
mu_test_begin(test_vartype_scanner_ext5)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_5.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(43, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(45, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(63, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing a logical AND.
 */
mu_test_begin(test_vartype_scanner_ext6)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_6.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_YZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(33, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(51, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(53, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(71, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing a logical OR.
 */
mu_test_begin(test_vartype_scanner_ext7)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_7.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_YZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(33, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(51, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(53, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(71, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing an equality expression.
 */
mu_test_begin(test_vartype_scanner_ext8)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_8.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(50, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(52, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a condition
 * containing an inequality expression.
 */
mu_test_begin(test_vartype_scanner_ext9)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_9.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(23, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(30, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(50, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(52, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing a commented extension with a complex
 * condition.
 */
mu_test_begin(test_vartype_scanner_ext10)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_10.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(23, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_YZ", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(25, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(30, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(32, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(33, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("def", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(35, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(39, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(41, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(42, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(44, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(44, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(45, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(45, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_A", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(46, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(50, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(51, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(51, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(53, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(54, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(56, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(56, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("yes|no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(57, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(64, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(65, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(65, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(66, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(66, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(68, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(68, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(70, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(73, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(75, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(75, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(77, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(95, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(97, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(115, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing an invalid quoted string.
 */
mu_test_begin(test_vartype_scanner_ext11)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_ext_11.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(13, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(22, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(24, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(49, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(51, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(69, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing multiple definitions.
 */
mu_test_begin(test_vartype_scanner_multi1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_multi_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a number", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NUMERIC", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(48, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing multiple definitions with comments
 * interspersed.
 */
mu_test_begin(test_vartype_scanner_multi2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_multi_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a number", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NUMERIC", token->value.str);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(48, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing multiple definitions and extensions with
 * comments interspersed.
 */
mu_test_begin(test_vartype_scanner_multi3)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_multi_3.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a number", token->value.qstr->value);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NUMERIC", token->value.str);
    mu_assert_eq_int(4, token->location.first_line);
    mu_assert_eq_int(48, token->location.first_column);
    mu_assert_eq_int(4, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EXT, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_XY", token->value.id->name);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("a\"b'\\\\c\"d", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(28, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(29, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(31, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(31, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(33, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(36, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(38, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(38, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(40, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(58, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("extension of NONE", token->value.str);
    mu_assert_eq_int(6, token->location.first_line);
    mu_assert_eq_int(60, token->location.first_column);
    mu_assert_eq_int(6, token->location.last_line);
    mu_assert_eq_int(78, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing multiple definitions of which the one in the
 * middle is syntactically incorrect (1).
 */
mu_test_begin(test_vartype_scanner_multi_wrong1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_multi_wrong_1.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(FALSE, token->value.id->valid);
    mu_assert_eq_str("AbC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("AbC", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected AbC", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of AbC", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(47, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a number", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NUMERIC", token->value.str);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(48, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning file containing multiple definitions of which the one in the
 * middle is syntactically incorrect (2).
 */
mu_test_begin(test_vartype_scanner_multi_wrong2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct vartype_token_t);

    package_init_module();
    result = run_scanner("grammar/tests/input/vartype_multi_wrong_2.txt");
    init_array_iterator(&it, result);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NONE", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(5, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str(".*", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(7, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected anything", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(32, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NONE", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(34, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(53, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("ABC", token->value.id->name);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(4, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("ABC", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(10, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str(";", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(12, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("expected ABC", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(27, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of ABC", token->value.str);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(29, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(47, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("NUMERIC", token->value.id->name);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQ, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(10, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(24, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COLON, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(26, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(26, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("expected a number", token->value.qstr->value);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(28, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(46, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(COMMENT, token->type);
    mu_assert_eq_str("definition of NUMERIC", token->value.str);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(48, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(70, token->location.last_column);

    token = (struct vartype_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

void
test_vartype_scanner(void)
{
    mu_run_test(test_vartype_scanner_empty_file);
    mu_run_test(test_vartype_scanner_comment1);
    mu_run_test(test_vartype_scanner_comment2);
    mu_run_test(test_vartype_scanner_def1);
    mu_run_test(test_vartype_scanner_def2);
    mu_run_test(test_vartype_scanner_def3);
    mu_run_test(test_vartype_scanner_def4);
    mu_run_test(test_vartype_scanner_def_invalid_id1);
    mu_run_test(test_vartype_scanner_def_invalid_id2);
    mu_run_test(test_vartype_scanner_def_invalid_eq);
    mu_run_test(test_vartype_scanner_def_invalid_regex);
    mu_run_test(test_vartype_scanner_def_invalid_colon);
    mu_run_test(test_vartype_scanner_def_invalid_errmsg);
    mu_run_test(test_vartype_scanner_def_comment1);
    mu_run_test(test_vartype_scanner_def_comment2);
    mu_run_test(test_vartype_scanner_def_comment3a);
    mu_run_test(test_vartype_scanner_def_comment3b);
    mu_run_test(test_vartype_scanner_string_multiline1a);
    mu_run_test(test_vartype_scanner_string_multiline1b);
    mu_run_test(test_vartype_scanner_string_multiline2a);
    mu_run_test(test_vartype_scanner_string_multiline2b);
    mu_run_test(test_vartype_scanner_string_multiline3a);
    mu_run_test(test_vartype_scanner_string_multiline3b);
    mu_run_test(test_vartype_scanner_string_multiline4a);
    mu_run_test(test_vartype_scanner_string_multiline4b);
    mu_run_test(test_vartype_scanner_string_multiline5a);
    mu_run_test(test_vartype_scanner_string_multiline5b);
    mu_run_test(test_vartype_scanner_string_multiline6a);
    mu_run_test(test_vartype_scanner_string_multiline6b);
    mu_run_test(test_vartype_scanner_string_multiline7a);
    mu_run_test(test_vartype_scanner_string_multiline7b);
    mu_run_test(test_vartype_scanner_string_multiline8a);
    mu_run_test(test_vartype_scanner_string_multiline8b);
    mu_run_test(test_vartype_scanner_string_multiline9a);
    mu_run_test(test_vartype_scanner_string_multiline9b);
    mu_run_test(test_vartype_scanner_string_multiline10a);
    mu_run_test(test_vartype_scanner_string_multiline10b);
    mu_run_test(test_vartype_scanner_string_open1);
    mu_run_test(test_vartype_scanner_string_open2);
    mu_run_test(test_vartype_scanner_string_open3);
    mu_run_test(test_vartype_scanner_string_open4);
    mu_run_test(test_vartype_scanner_string_open5);
    mu_run_test(test_vartype_scanner_string_open6);
    mu_run_test(test_vartype_scanner_string_open7);
    mu_run_test(test_vartype_scanner_string_escape1);
    mu_run_test(test_vartype_scanner_string_escape2);
    mu_run_test(test_vartype_scanner_string_escape3);
    mu_run_test(test_vartype_scanner_string_escape4);
    mu_run_test(test_vartype_scanner_string_escape5);
    mu_run_test(test_vartype_scanner_string_escape6);
    mu_run_test(test_vartype_scanner_string_escape7);
    mu_run_test(test_vartype_scanner_bom);
    mu_run_test(test_vartype_scanner_tabs);
    mu_run_test(test_vartype_scanner_ext1);
    mu_run_test(test_vartype_scanner_ext2);
    mu_run_test(test_vartype_scanner_ext3);
    mu_run_test(test_vartype_scanner_ext4a);
    mu_run_test(test_vartype_scanner_ext4b);
    mu_run_test(test_vartype_scanner_ext5);
    mu_run_test(test_vartype_scanner_ext6);
    mu_run_test(test_vartype_scanner_ext7);
    mu_run_test(test_vartype_scanner_ext8);
    mu_run_test(test_vartype_scanner_ext9);
    mu_run_test(test_vartype_scanner_ext10);
    mu_run_test(test_vartype_scanner_ext11);
    mu_run_test(test_vartype_scanner_multi1);
    mu_run_test(test_vartype_scanner_multi2);
    mu_run_test(test_vartype_scanner_multi3);
    mu_run_test(test_vartype_scanner_multi_wrong1);
    mu_run_test(test_vartype_scanner_multi_wrong2);
}
