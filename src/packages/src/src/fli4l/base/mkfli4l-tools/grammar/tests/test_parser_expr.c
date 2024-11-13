/*****************************************************************************
 *  @file grammar/tests/test_parser_expr.c
 *  Functions for testing the parsing of expressions.
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
#include <stdlib.h>
#include <string.h>
#include "grammar/expr.h"
#include "grammar/expr_scanner.h"
#include "package.h"
#include "var.h"

#define MU_TESTSUITE "grammar.expr.parser"

#define TEST_PACKAGE "test"

/**
 * Runs the scanner and parser for a given expression.
 *
 * @param expr
 *  The expression string.
 * @param exit_code
 *  Receives the exit code of the parser.
 * @return
 *  The parsed expression tree.
 */
static struct expr_parse_tree_t *
run_scanner_and_parser(char const *expr, int *exit_code)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, "package.txt");
    struct expr_parse_tree_t *tree = expr_parse_tree_create();
    struct var_scope_t *scope = var_get_global_scope();
    yyscan_t scanner;

    if (expr_lex_init_extra(file, &scanner) != 0) {
        log_error("Error initializing scanner for expression %s\n", expr);
    }
    else {
        YY_BUFFER_STATE state = expr__scan_string(expr, scanner);
        state->yy_bs_lineno = 1;
        state->yy_bs_column = 0;
        *exit_code = expr_parse(scanner, file, tree, scope);
        expr__delete_buffer(state, scanner);
        expr_lex_destroy(scanner);
    }

    return tree;
}

/**
 * Tests parsing empty string.
 */
mu_test_begin(test_expr_parser_empty_string)
    struct expr_parse_tree_t *tree;
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("", &exit_code);

    mu_assert(!tree->ok);
    mu_assert(!tree->expr);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing quoted strings (1).
 */
mu_test_begin(test_expr_parser_string1)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("\"no\"", &exit_code);

    mu_assert(tree->ok);

    expected = expr_literal_to_expr(expr_literal_create_string(scope, "no"));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing quoted strings (2).
 */
mu_test_begin(test_expr_parser_string2)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("'no'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_literal_to_expr(expr_literal_create_string(scope, "no"));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing incorrectly quoted strings (1).
 */
mu_test_begin(test_expr_parser_string_invalid1)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("\"no", &exit_code);

    mu_assert(!tree->ok);

    expected = expr_literal_to_expr(expr_literal_create_string(scope, "no"));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing incorrectly quoted strings (2).
 */
mu_test_begin(test_expr_parser_string_invalid2)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("'no", &exit_code);

    mu_assert(!tree->ok);

    expected = expr_literal_to_expr(expr_literal_create_string(scope, "no"));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing variable expressions.
 */
mu_test_begin(test_expr_parser_var_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC", &exit_code);

    mu_assert(tree->ok);

    expected = expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC"));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing logical NOT expressions.
 */
mu_test_begin(test_expr_parser_logical_not_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!!OPT_ABC", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_not_to_expr(expr_logical_not_create(scope,
        expr_logical_not_to_expr(expr_logical_not_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing logical AND expressions.
 */
mu_test_begin(test_expr_parser_logical_and_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC && OPT_DEF", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_and_to_expr(expr_logical_and_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing logical OR expressions.
 */
mu_test_begin(test_expr_parser_logical_or_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC || OPT_DEF", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing equality expressions.
 */
mu_test_begin(test_expr_parser_equal_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("\"yes\" == 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing match expressions (1).
 */
mu_test_begin(test_expr_parser_match_expr1)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC =~ \"yes|no\"", &exit_code);

    mu_assert(tree->ok);

    expected = expr_match_to_expr(expr_match_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing match expressions (2).
 */
mu_test_begin(test_expr_parser_match_expr2)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC =~ 'yes|no'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_match_to_expr(expr_match_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing inequality expressions.
 */
mu_test_begin(test_expr_parser_unequal_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC != OPT_DEF", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing bracket expressions.
 */
mu_test_begin(test_expr_parser_bracket_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("((OPT_ABC) == ('abc'))", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_literal_to_expr(expr_literal_create_string(scope, "abc"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing expressions spanning multiple lines and using various
 * whitespace characters.
 */
mu_test_begin(test_expr_parser_multiline_expr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("((OPT_ABC)\n\t==\t(\v'abc' \r\n))", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_literal_to_expr(expr_literal_create_string(scope, "abc"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing expressions with invalid characters.
 */
mu_test_begin(test_expr_parser_invalid_expr)
    struct expr_parse_tree_t *tree;
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("(OPT_AB$ != x) == ('abc' != ABC)", &exit_code);

    mu_assert(!tree->ok);
    mu_assert(!tree->expr);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT before ==.
 */
mu_test_begin(test_expr_parser_logical_not_before_equal)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!OPT_ABC == 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_logical_not_to_expr(expr_logical_not_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT after == (using brackets).
 */
mu_test_begin(test_expr_parser_logical_not_after_equal)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!(OPT_ABC == 'yes')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_not_to_expr(expr_logical_not_create(scope,
        expr_equal_to_expr(expr_equal_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT before =~.
 */
mu_test_begin(test_expr_parser_logical_not_before_match)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!OPT_ABC =~ 'yes|no'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_match_to_expr(expr_match_create(scope,
        expr_logical_not_to_expr(expr_logical_not_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT after =~ (using brackets).
 */
mu_test_begin(test_expr_parser_logical_not_after_match)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!(OPT_ABC =~ 'yes|no')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_not_to_expr(expr_logical_not_create(scope,
        expr_match_to_expr(expr_match_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT before !=.
 */
mu_test_begin(test_expr_parser_logical_not_before_unequal)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!OPT_ABC != 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_logical_not_to_expr(expr_logical_not_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing NOT after != (using brackets).
 */
mu_test_begin(test_expr_parser_logical_not_after_unequal)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("!(OPT_ABC != 'yes')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_not_to_expr(expr_logical_not_create(scope,
        expr_unequal_to_expr(expr_unequal_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing == left-to-right.
 */
mu_test_begin(test_expr_parser_equal_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == 'yes' == 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_equal_to_expr(expr_equal_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing == right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_equal_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == ('yes' == 'no')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_equal_to_expr(expr_equal_create(scope,
            expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
            expr_literal_to_expr(expr_literal_create_string(scope, "no"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing =~ left-to-right.
 */
mu_test_begin(test_expr_parser_match_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC =~ 'yes|no' =~ 'yes|no'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_match_to_expr(expr_match_create(scope,
        expr_match_to_expr(expr_match_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing =~ right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_match_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC =~ ('yes' =~ 'yes|no')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_match_to_expr(expr_match_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_match_to_expr(expr_match_create(scope,
            expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing != left-to-right.
 */
mu_test_begin(test_expr_parser_unequal_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC != 'yes' != 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_unequal_to_expr(expr_unequal_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing != right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_unequal_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC != ('yes' != 'no')", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_unequal_to_expr(expr_unequal_create(scope,
            expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
            expr_literal_to_expr(expr_literal_create_string(scope, "no"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing ==, =~, != left-to-right.
 */
mu_test_begin(test_expr_parser_rel_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == 'yes' =~ 'yes|no' != 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_match_to_expr(expr_match_create(scope,
            expr_equal_to_expr(expr_equal_create(scope,
                expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
                expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
            )),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing ==, =~, != right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_rel_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == ('yes' =~ ('no' != 'yes'))", &exit_code);

    mu_assert(tree->ok);

    expected = expr_equal_to_expr(expr_equal_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_match_to_expr(expr_match_create(scope,
            expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
            expr_unequal_to_expr(expr_unequal_create(scope,
                expr_literal_to_expr(expr_literal_create_string(scope, "no")),
                expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
            ))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing ==, =~, != before AND.
 */
mu_test_begin(test_expr_parser_rel_before_logical_and)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == 'yes' && OPT_DEF =~ 'yes|no' && OPT_GHI != 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_and_to_expr(expr_logical_and_create(scope,
        expr_logical_and_to_expr(expr_logical_and_create(scope,
            expr_equal_to_expr(expr_equal_create(scope,
                expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
                expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
            )),
            expr_match_to_expr(expr_match_create(scope,
                expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF")),
                expr_literal_to_expr(expr_literal_create_string(scope, "yes|no"))
            ))
        )),
        expr_unequal_to_expr(expr_unequal_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI")),
            expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing ==, =~, != after AND (using brackets).
 */
mu_test_begin(test_expr_parser_rel_after_logical_and)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC == ('yes' && OPT_DEF) =~ ('no' && OPT_GHI) != 'yes'", &exit_code);

    mu_assert(tree->ok);

    expected = expr_unequal_to_expr(expr_unequal_create(scope,
        expr_match_to_expr(expr_match_create(scope,
            expr_equal_to_expr(expr_equal_create(scope,
                expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
                expr_logical_and_to_expr(expr_logical_and_create(scope,
                    expr_literal_to_expr(expr_literal_create_string(scope, "yes")),
                    expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
                ))
            )),
            expr_logical_and_to_expr(expr_logical_and_create(scope,
                expr_literal_to_expr(expr_literal_create_string(scope, "no")),
                expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
            ))
        )),
        expr_literal_to_expr(expr_literal_create_string(scope, "yes"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing AND left-to-right.
 */
mu_test_begin(test_expr_parser_logical_and_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC && OPT_DEF && OPT_GHI", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_and_to_expr(expr_logical_and_create(scope,
        expr_logical_and_to_expr(expr_logical_and_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
        )),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing AND right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_logical_and_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC && (OPT_DEF && OPT_GHI)", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_and_to_expr(expr_logical_and_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_logical_and_to_expr(expr_logical_and_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing AND before OR.
 */
mu_test_begin(test_expr_parser_logical_and_before_logical_or)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC || OPT_DEF && OPT_GHI", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_logical_and_to_expr(expr_logical_and_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing AND after OR (using brackets).
 */
mu_test_begin(test_expr_parser_logical_and_after_logical_or)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("(OPT_ABC || OPT_DEF) && OPT_GHI", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_and_to_expr(expr_logical_and_create(scope,
        expr_logical_or_to_expr(expr_logical_or_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
        )),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing OR left-to-right.
 */
mu_test_begin(test_expr_parser_logical_or_ltr)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC || OPT_DEF || OPT_GHI", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_logical_or_to_expr(expr_logical_or_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF"))
        )),
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing OR right-to-left (using brackets).
 */
mu_test_begin(test_expr_parser_logical_or_rtl)
    struct expr_parse_tree_t *tree;
    struct expr_t *expected;
    struct var_scope_t *scope = var_get_global_scope();
    int exit_code;

    package_init_module();
    tree = run_scanner_and_parser("OPT_ABC || (OPT_DEF || OPT_GHI)", &exit_code);

    mu_assert(tree->ok);

    expected = expr_logical_or_to_expr(expr_logical_or_create(scope,
        expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_ABC")),
        expr_logical_or_to_expr(expr_logical_or_create(scope,
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_DEF")),
            expr_variable_to_expr(expr_variable_create_by_name(scope, "OPT_GHI"))
        ))
    ));
    mu_assert(expr_equal(expected, tree->expr));
    expr_destroy(expected);

    expr_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

void
test_expr_parser(void)
{
    mu_run_test(test_expr_parser_empty_string);
    mu_run_test(test_expr_parser_string1);
    mu_run_test(test_expr_parser_string2);
    mu_run_test(test_expr_parser_string_invalid1);
    mu_run_test(test_expr_parser_string_invalid2);
    mu_run_test(test_expr_parser_var_expr);
    mu_run_test(test_expr_parser_logical_not_expr);
    mu_run_test(test_expr_parser_logical_and_expr);
    mu_run_test(test_expr_parser_logical_or_expr);
    mu_run_test(test_expr_parser_equal_expr);
    mu_run_test(test_expr_parser_match_expr1);
    mu_run_test(test_expr_parser_match_expr2);
    mu_run_test(test_expr_parser_unequal_expr);
    mu_run_test(test_expr_parser_bracket_expr);
    mu_run_test(test_expr_parser_multiline_expr);
    mu_run_test(test_expr_parser_invalid_expr);
    mu_run_test(test_expr_parser_logical_not_before_equal);
    mu_run_test(test_expr_parser_logical_not_after_equal);
    mu_run_test(test_expr_parser_logical_not_before_match);
    mu_run_test(test_expr_parser_logical_not_after_match);
    mu_run_test(test_expr_parser_logical_not_before_unequal);
    mu_run_test(test_expr_parser_logical_not_after_unequal);
    mu_run_test(test_expr_parser_equal_ltr);
    mu_run_test(test_expr_parser_equal_rtl);
    mu_run_test(test_expr_parser_match_ltr);
    mu_run_test(test_expr_parser_match_rtl);
    mu_run_test(test_expr_parser_unequal_ltr);
    mu_run_test(test_expr_parser_unequal_rtl);
    mu_run_test(test_expr_parser_rel_ltr);
    mu_run_test(test_expr_parser_rel_rtl);
    mu_run_test(test_expr_parser_rel_before_logical_and);
    mu_run_test(test_expr_parser_rel_after_logical_and);
    mu_run_test(test_expr_parser_logical_and_ltr);
    mu_run_test(test_expr_parser_logical_and_rtl);
    mu_run_test(test_expr_parser_logical_and_before_logical_or);
    mu_run_test(test_expr_parser_logical_and_after_logical_or);
    mu_run_test(test_expr_parser_logical_or_ltr);
    mu_run_test(test_expr_parser_logical_or_rtl);
}
