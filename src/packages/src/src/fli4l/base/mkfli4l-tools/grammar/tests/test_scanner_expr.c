/*****************************************************************************
 *  @file grammar/tests/test_scanner_expr.c
 *  Functions for testing the scanning of expressions.
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

#include <testing/tests.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/array.h>
#include <stdlib.h>
#include <string.h>
#include "expression.h"
#include "grammar/expr_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.expr.scanner"

#define TEST_PACKAGE "test"

/**
 * Represents a token recognized by the scanner.
 */
struct expr_token_t {
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
 * Runs the scanner for a given expression.
 *
 * @param expr
 *  The expression string.
 * @return
 *  An array of tokens which have been recognized.
 */
static array_t *
run_scanner(char const *expr)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, "package.txt");
    array_t *result = init_array(32);
    yyscan_t scanner;

    if (expr_lex_init_extra(file, &scanner) != 0) {
        log_error("Error initializing scanner for expression %s\n", expr);
    }
    else {
        YY_BUFFER_STATE state = expr__scan_string(expr, scanner);
        state->yy_bs_lineno = 1;
        state->yy_bs_column = 0;

        while (TRUE) {
            struct expr_token_t *token = (struct expr_token_t *)
                safe_malloc(sizeof(struct expr_token_t));
            token->type
                = expr_lex(&token->value, &token->location, scanner);
            append_new_elem(result, token);
            if (token->type == 0) {
                break;
            }
        }

        expr__delete_buffer(state, scanner);
        expr_lex_destroy(scanner);
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
    ARRAY_ITER(it, array, token, struct expr_token_t) {
        switch (token->type) {
        case ID :
            identifier_destroy(token->value.id);
            break;
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
 * Tests scanning empty string.
 */
mu_test_begin(test_expr_scanner_empty_string)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning quoted strings (1).
 */
mu_test_begin(test_expr_scanner_string1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("\"no\"");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning quoted strings (2).
 */
mu_test_begin(test_expr_scanner_string2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("'no'");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(3, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning incorrectly quoted strings (1).
 */
mu_test_begin(test_expr_scanner_string_invalid1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("\"no");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning incorrectly quoted strings (2).
 */
mu_test_begin(test_expr_scanner_string_invalid2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("'no");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(FALSE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(2, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning variable expressions.
 */
mu_test_begin(test_expr_scanner_var_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning logical NOT expressions.
 */
mu_test_begin(test_expr_scanner_logical_not_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("!!OPT_ABC");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(NOT, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning logical AND expressions.
 */
mu_test_begin(test_expr_scanner_logical_and_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC && OPT_DEF");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(AND, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_DEF", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning logical OR expressions.
 */
mu_test_begin(test_expr_scanner_logical_or_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC || OPT_DEF");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(OR, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_DEF", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning equality expressions.
 */
mu_test_begin(test_expr_scanner_equal_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("\"yes\" == 'yes'");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("yes", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(4, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(7, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("yes", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(13, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning match expressions (1).
 */
mu_test_begin(test_expr_scanner_match_expr1)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC =~ \"yes|no\"");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('"', token->value.qstr->delim);
    mu_assert_eq_str("yes|no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning match expressions (2).
 */
mu_test_begin(test_expr_scanner_match_expr2)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC =~ 'yes|no'");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(MATCH, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("yes|no", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(18, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning inequality expressions.
 */
mu_test_begin(test_expr_scanner_unequal_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_ABC != OPT_DEF");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_DEF", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(17, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning bracket expressions.
 */
mu_test_begin(test_expr_scanner_bracket_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("((OPT_ABC) == ('abc'))");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(12, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(14, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(14, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(15, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(19, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(20, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(20, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(21, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(21, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning expressions spanning multiple lines and using various
 * whitespace characters.
 */
mu_test_begin(test_expr_scanner_multiline_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("((OPT_ABC)\n\t==\t(\v'abc' \r\n))");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_ABC", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(2, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(8, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(9, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(EQUAL, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(LBRACKET, token->type);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(16, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(16, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(QSTRING, token->type);
    mu_assert_eq_int(TRUE, token->value.qstr->valid);
    mu_assert_eq_int('\'', token->value.qstr->delim);
    mu_assert_eq_str("abc", token->value.qstr->value);
    mu_assert_eq_int(2, token->location.first_line);
    mu_assert_eq_int(18, token->location.first_column);
    mu_assert_eq_int(2, token->location.last_line);
    mu_assert_eq_int(22, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(0, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(RBRACKET, token->type);
    mu_assert_eq_int(3, token->location.first_line);
    mu_assert_eq_int(1, token->location.first_column);
    mu_assert_eq_int(3, token->location.last_line);
    mu_assert_eq_int(1, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

/**
 * Tests scanning expressions with invalid characters.
 */
mu_test_begin(test_expr_scanner_invalid_expr)
    array_t *result;
    DECLARE_ARRAY_ITER(it, token, struct expr_token_t);

    package_init_module();
    result = run_scanner("OPT_AB$ != x");
    init_array_iterator(&it, result);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(ID, token->type);
    mu_assert_eq_int(TRUE, token->value.id->valid);
    mu_assert_eq_str("OPT_AB", token->value.id->name);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(0, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(5, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str("$", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(6, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(6, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNEQUAL, token->type);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(8, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(9, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(UNKNOWN, token->type);
    mu_assert_eq_str("x", token->value.str);
    mu_assert_eq_int(1, token->location.first_line);
    mu_assert_eq_int(11, token->location.first_column);
    mu_assert_eq_int(1, token->location.last_line);
    mu_assert_eq_int(11, token->location.last_column);

    token = (struct expr_token_t *) get_next_elem(&it);
    mu_assert(token);
    mu_assert_eq_int(0, token->type);

    free_tokens(result);
    package_fini_module();
mu_test_end()

void
test_expr_scanner(void)
{
    mu_run_test(test_expr_scanner_empty_string);
    mu_run_test(test_expr_scanner_string1);
    mu_run_test(test_expr_scanner_string2);
    mu_run_test(test_expr_scanner_string_invalid1);
    mu_run_test(test_expr_scanner_string_invalid2);
    mu_run_test(test_expr_scanner_var_expr);
    mu_run_test(test_expr_scanner_logical_not_expr);
    mu_run_test(test_expr_scanner_logical_and_expr);
    mu_run_test(test_expr_scanner_logical_or_expr);
    mu_run_test(test_expr_scanner_equal_expr);
    mu_run_test(test_expr_scanner_match_expr1);
    mu_run_test(test_expr_scanner_match_expr2);
    mu_run_test(test_expr_scanner_unequal_expr);
    mu_run_test(test_expr_scanner_bracket_expr);
    mu_run_test(test_expr_scanner_multiline_expr);
    mu_run_test(test_expr_scanner_invalid_expr);
}
