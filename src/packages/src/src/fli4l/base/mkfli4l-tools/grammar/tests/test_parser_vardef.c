/*****************************************************************************
 *  @file grammar/tests/test_parser_vardef.c
 *  Functions for testing the parsing of variable definition files.
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
#include <expression.h>
#include <stdlib.h>
#include <string.h>
#include "grammar/vardef.h"
#include "grammar/vardef_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.vardef.parser"

#define TEST_PACKAGE "test"

/**
 * Runs the scanner and parser for a given input file.
 *
 * @param name
 *  The name of the input file.
 * @param exit_code
 *  Receives the exit code of the parser.
 * @return
 *  An array of tokens which have been recognized.
 */
static struct vardef_parse_tree_t *
run_scanner_and_parser(char const *name, int *exit_code)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_VARDEF, name);
    struct vardef_parse_tree_t *tree = vardef_parse_tree_create();

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (vardef_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", name);
        }
        else {
            vardef_set_in(f, scanner);
            *exit_code = vardef_parse(scanner, file, tree);
            vardef_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable definition file %s\n", name);
    }

    return tree;
}

/**
 * Tests parsing empty file.
 */
mu_test_begin(test_vardef_parser_empty_file)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/empty.txt", &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (1).
 */
mu_test_begin(test_vardef_parser_comment1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/comment_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (2).
 */
mu_test_begin(test_vardef_parser_comment2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/comment_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition only.
 */
mu_test_begin(test_vardef_parser_def1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition.
 */
mu_test_begin(test_vardef_parser_def2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition without a
 * default value.
 */
mu_test_begin(test_vardef_parser_def3)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of an optional
 * variable.
 */
mu_test_begin(test_vardef_parser_def4)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_4.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_4.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of a really
 * optional array.
 */
mu_test_begin(test_vardef_parser_def5)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_5.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1_%_XY", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1_%_XY", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of a really
 * optional array.
 */
mu_test_begin(test_vardef_parser_def6)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_6.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1_N", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1_%_XY", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1_%_XY", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_6.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing two definitions of which one variable depends
 * on the other one.
 */
mu_test_begin(test_vardef_parser_def7a)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("YESNO", def->type->u.id->name);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7a.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR2", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_variable_to_expr(expr_variable_create_by_name(
        var_get_global_scope(),
        "VAR1"
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing two definitions of which one variable depends
 * on the inverse of another one.
 */
mu_test_begin(test_vardef_parser_def7b)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("YESNO", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7b.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR2", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "VAR1"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using an enabling condition with a
 * logical AND.
 */
mu_test_begin(test_vardef_parser_def7c)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7c.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_Y"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7c.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using an enabling condition with a
 * logical OR.
 */
mu_test_begin(test_vardef_parser_def7d)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7d.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_Y"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7d.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using an enabling condition with an
 * equality expression.
 */
mu_test_begin(test_vardef_parser_def7e)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7e.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7e.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using an enabling condition with an
 * inequality expression.
 */
mu_test_begin(test_vardef_parser_def7f)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7f.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7f.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using a complex enabling condition.
 */
mu_test_begin(test_vardef_parser_def7g)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7g.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(def->cond->valid);
    expected = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        expr_logical_and_to_expr(expr_logical_and_create(
            var_get_global_scope(),
            expr_unequal_to_expr(expr_unequal_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_XY"
                )),
                expr_literal_to_expr(expr_literal_create_string(
                    var_get_global_scope(),
                    "abc"
                ))
            )),
            expr_equal_to_expr(expr_equal_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_YZ"
                )),
                expr_literal_to_expr(expr_literal_create_string(
                    var_get_global_scope(),
                    "def"
                ))
            ))
        )),
        expr_match_to_expr(expr_match_create(
            var_get_global_scope(),
            expr_logical_not_to_expr(expr_logical_not_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_A"
                ))
            )),
            expr_literal_to_expr(expr_literal_create_string(
                var_get_global_scope(),
                "yes|no"
            ))
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7g.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a variable using a condition with an invalid
 * quoted string.
 */
mu_test_begin(test_vardef_parser_def7h)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_7h.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("V", def->id->name);
    mu_assert(def->cond);
    mu_assert(!def->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, def->cond->expr));
    expr_destroy(expected);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_7h.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a definition with an anonymous variable type.
 */
mu_test_begin(test_vardef_parser_def8)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_8.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(FALSE, def->type->named);
    mu_assert_eq_str("abc|(def)+|(e*fg*)?", def->type->u.regex);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc\\"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_8.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_vardef_parser_def_comment1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_comment_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_comment_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition and comments around it.
 */
mu_test_begin(test_vardef_parser_def_comment2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_comment_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_comment_2.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a multi-line commented single definition,
 * comments above and below, and LF line endings.
 */
mu_test_begin(test_vardef_parser_def_comment3a)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_comment_3a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_comment_3a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a multi-line commented single definition,
 * comments above and below, and CR/LF line endings.
 */
mu_test_begin(test_vardef_parser_def_comment3b)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_comment_3b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_comment_3b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (1).
 */
mu_test_begin(test_vardef_parser_string_escape1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "\\abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (2).
 */
mu_test_begin(test_vardef_parser_string_escape2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a\\bc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (3).
 */
mu_test_begin(test_vardef_parser_string_escape3)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc\\"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (4).
 */
mu_test_begin(test_vardef_parser_string_escape4)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_4.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "\\abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_4.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (5).
 */
mu_test_begin(test_vardef_parser_string_escape5)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_5.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a\\bc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (6).
 */
mu_test_begin(test_vardef_parser_string_escape6)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_6.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc\\"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (7).
 */
mu_test_begin(test_vardef_parser_string_escape7)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_string_escape_7.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "\\\"abc\"\''\""));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_string_escape_7.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a wrong identifier.
 */
mu_test_begin(test_vardef_parser_def_wrong1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_wrong_1.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("VaR1_%_XY", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VaR1_%_XY", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_wrong_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("VAr1_%_XY", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAr1_%_XY", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_wrong_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("var1_%_xy", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of var1_%_xy", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_wrong_1.txt' (package test) 3:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a wrong condition.
 */
mu_test_begin(test_vardef_parser_def_wrong2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_wrong_2.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an unclosed string.
 */
mu_test_begin(test_vardef_parser_def_wrong3)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_def_wrong_3.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1_%_XY", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(!def->defvalue->valid);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_def_wrong_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition with a BOM.
 */
mu_test_begin(test_vardef_parser_bom)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_bom.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_bom.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition with TABs.
 */
mu_test_begin(test_vardef_parser_tabs)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tabs.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a\tbc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of\tVAR1", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tabs.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition only.
 */
mu_test_begin(test_vardef_parser_tmpldef1)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition.
 */
mu_test_begin(test_vardef_parser_tmpldef2)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NONE", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR.+", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition without a
 * default value.
 */
mu_test_begin(test_vardef_parser_tmpldef3)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("definition of VAR.+", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of an optional
 * variable.
 */
mu_test_begin(test_vardef_parser_tmpldef4)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_4.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR.+", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_4.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of a really
 * optional array.
 */
mu_test_begin(test_vardef_parser_tmpldef5)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_5.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(TRUE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR1_%_.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1_%_.+", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single commented definition of a really
 * optional array.
 */
mu_test_begin(test_vardef_parser_tmpldef6)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_6.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1_N", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(TRUE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%VAR1_%_.+", tmpldef->id->name);
    mu_assert(!tmpldef->cond);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_integer(var_get_global_scope(), 123));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR1_%_.+", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_6.txt' (package test) 2:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing two definitions of which one variable depends
 * on the other one.
 */
mu_test_begin(test_vardef_parser_tmpldef7a)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("YESNO", def->type->u.id->name);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7a.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%VAR.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    mu_assert(tmpldef->cond->valid);
    expected = expr_variable_to_expr(expr_variable_create_by_name(
        var_get_global_scope(),
        "VAR1"
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7a.txt' (package test) 2:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing two definitions of which one variable depends
 * on the inverse of another one.
 */
mu_test_begin(test_vardef_parser_tmpldef7b)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("VAR1", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(TRUE, def->type->named);
    mu_assert(def->type->u.id->valid);
    mu_assert_eq_str("YESNO", def->type->u.id->name);
    mu_assert(!def->defvalue);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7b.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%VAR.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "VAR1"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7b.txt' (package test) 2:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using an enabling
 * condition with a logical AND.
 */
mu_test_begin(test_vardef_parser_tmpldef7c)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7c.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_Y"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7c.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using an enabling
 * condition with a logical OR.
 */
mu_test_begin(test_vardef_parser_tmpldef7d)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7d.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_Y"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7d.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using an enabling
 * condition with an equality expression.
 */
mu_test_begin(test_vardef_parser_tmpldef7e)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7e.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7e.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using an enabling
 * condition with an inequality expression.
 */
mu_test_begin(test_vardef_parser_tmpldef7f)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7f.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7f.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using a complex enabling
 * condition.
 */
mu_test_begin(test_vardef_parser_tmpldef7g)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7g.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(tmpldef->cond->valid);
    expected = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        expr_logical_and_to_expr(expr_logical_and_create(
            var_get_global_scope(),
            expr_unequal_to_expr(expr_unequal_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_XY"
                )),
                expr_literal_to_expr(expr_literal_create_string(
                    var_get_global_scope(),
                    "abc"
                ))
            )),
            expr_equal_to_expr(expr_equal_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_YZ"
                )),
                expr_literal_to_expr(expr_literal_create_string(
                    var_get_global_scope(),
                    "def"
                ))
            ))
        )),
        expr_match_to_expr(expr_match_create(
            var_get_global_scope(),
            expr_logical_not_to_expr(expr_logical_not_create(
                var_get_global_scope(),
                expr_variable_to_expr(expr_variable_create_by_name(
                    var_get_global_scope(),
                    "OPT_A"
                ))
            )),
            expr_literal_to_expr(expr_literal_create_string(
                var_get_global_scope(),
                "yes|no"
            ))
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7g.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition using a condition with
 * an invalid quoted string.
 */
mu_test_begin(test_vardef_parser_tmpldef7h)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_t);
    DECLARE_ARRAY_ITER(it2, tmpldef, struct vardef_template_t);
    char *locstr;
    struct expr_t *expected;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_7h.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it, tree->definitions);
    def = (struct vardef_t *) get_next_elem(&it);
    mu_assert(!def);

    init_array_iterator(&it2, tree->template_definitions);
    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(tmpldef);
    mu_assert(!tmpldef->valid);
    mu_assert_eq_int(FALSE, tmpldef->optional);
    mu_assert(tmpldef->id->valid);
    mu_assert_eq_str("%V.+", tmpldef->id->name);
    mu_assert(tmpldef->cond);
    mu_assert(!tmpldef->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_X"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, tmpldef->cond->expr));
    expr_destroy(expected);
    mu_assert(tmpldef->type->valid);
    mu_assert_eq_int(TRUE, tmpldef->type->named);
    mu_assert(tmpldef->type->u.id->valid);
    mu_assert_eq_str("NUMERIC", tmpldef->type->u.id->name);
    mu_assert(tmpldef->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "123"));
    mu_assert(expr_equal(defvalue, tmpldef->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("", tmpldef->comment);
    locstr = location_toString(tmpldef->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_7h.txt' (package test) 1:0]", locstr);
    free(locstr);

    tmpldef = (struct vardef_template_t *) get_next_elem(&it2);
    mu_assert(!tmpldef);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a template definition with an anonymous
 * variable type.
 */
mu_test_begin(test_vardef_parser_tmpldef8)
    struct vardef_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, def, struct vardef_template_t);
    char *locstr;
    struct expr_t *defvalue;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vardef_tmpldef_8.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->template_definitions);
    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert_eq_int(FALSE, def->optional);
    mu_assert(def->id->valid);
    mu_assert_eq_str("%VAR.+", def->id->name);
    mu_assert(!def->cond);
    mu_assert(def->type->valid);
    mu_assert_eq_int(FALSE, def->type->named);
    mu_assert_eq_str("abc|(def)+|(e*fg*)?", def->type->u.regex);
    mu_assert(def->defvalue->valid);
    defvalue = expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc\\"));
    mu_assert(expr_equal(defvalue, def->defvalue->expr));
    expr_destroy(defvalue);
    mu_assert_eq_str("definition of VAR.+", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable definition file 'grammar/tests/input/vardef_tmpldef_8.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vardef_template_t *) get_next_elem(&it);
    mu_assert(!def);

    vardef_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

void
test_vardef_parser(void)
{
    mu_run_test(test_vardef_parser_empty_file);
    mu_run_test(test_vardef_parser_comment1);
    mu_run_test(test_vardef_parser_comment2);
    mu_run_test(test_vardef_parser_def1);
    mu_run_test(test_vardef_parser_def2);
    mu_run_test(test_vardef_parser_def3);
    mu_run_test(test_vardef_parser_def4);
    mu_run_test(test_vardef_parser_def5);
    mu_run_test(test_vardef_parser_def6);
    mu_run_test(test_vardef_parser_def7a);
    mu_run_test(test_vardef_parser_def7b);
    mu_run_test(test_vardef_parser_def7c);
    mu_run_test(test_vardef_parser_def7d);
    mu_run_test(test_vardef_parser_def7e);
    mu_run_test(test_vardef_parser_def7f);
    mu_run_test(test_vardef_parser_def7g);
    mu_run_test(test_vardef_parser_def7h);
    mu_run_test(test_vardef_parser_def8);
    mu_run_test(test_vardef_parser_def_comment1);
    mu_run_test(test_vardef_parser_def_comment2);
    mu_run_test(test_vardef_parser_def_comment3a);
    mu_run_test(test_vardef_parser_def_comment3b);
    mu_run_test(test_vardef_parser_string_escape1);
    mu_run_test(test_vardef_parser_string_escape2);
    mu_run_test(test_vardef_parser_string_escape3);
    mu_run_test(test_vardef_parser_string_escape4);
    mu_run_test(test_vardef_parser_string_escape5);
    mu_run_test(test_vardef_parser_string_escape6);
    mu_run_test(test_vardef_parser_string_escape7);
    mu_run_test(test_vardef_parser_def_wrong1);
    mu_run_test(test_vardef_parser_def_wrong2);
    mu_run_test(test_vardef_parser_def_wrong3);
    mu_run_test(test_vardef_parser_bom);
    mu_run_test(test_vardef_parser_tabs);
    mu_run_test(test_vardef_parser_tmpldef1);
    mu_run_test(test_vardef_parser_tmpldef2);
    mu_run_test(test_vardef_parser_tmpldef3);
    mu_run_test(test_vardef_parser_tmpldef4);
    mu_run_test(test_vardef_parser_tmpldef5);
    mu_run_test(test_vardef_parser_tmpldef6);
    mu_run_test(test_vardef_parser_tmpldef7a);
    mu_run_test(test_vardef_parser_tmpldef7b);
    mu_run_test(test_vardef_parser_tmpldef7c);
    mu_run_test(test_vardef_parser_tmpldef7d);
    mu_run_test(test_vardef_parser_tmpldef7e);
    mu_run_test(test_vardef_parser_tmpldef7f);
    mu_run_test(test_vardef_parser_tmpldef7g);
    mu_run_test(test_vardef_parser_tmpldef7h);
    mu_run_test(test_vardef_parser_tmpldef8);
}
