/*****************************************************************************
 *  @file grammar/tests/test_parser_vartype.c
 *  Functions for testing the parsing of variable type files.
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
#include <stdlib.h>
#include <string.h>
#include "grammar/vartype.h"
#include "grammar/vartype_scanner.h"
#include "package.h"

#define MU_TESTSUITE "grammar.vartype.parser"

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
static struct vartype_parse_tree_t *
run_scanner_and_parser(char const *name, int *exit_code)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, name);
    struct vartype_parse_tree_t *tree = vartype_parse_tree_create();

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (vartype_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", name);
        }
        else {
            vartype_set_in(f, scanner);
            *exit_code = vartype_parse(scanner, file, tree);
            vartype_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable types file %s\n", name);
    }

    return tree;
}

/**
 * Tests parsing an empty file.
 */
mu_test_begin(test_vartype_parser_empty_file)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/empty.txt", &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (1).
 */
mu_test_begin(test_vartype_parser_comment1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/comment_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (2).
 */
mu_test_begin(test_vartype_parser_comment2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/comment_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition only.
 */
mu_test_begin(test_vartype_parser_def1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition (1).
 */
mu_test_begin(test_vartype_parser_def2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition (2).
 */
mu_test_begin(test_vartype_parser_def3)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("TYPE123_XYZ", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines.
 */
mu_test_begin(test_vartype_parser_def4)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_4.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_4.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid identifier (1).
 */
mu_test_begin(test_vartype_parser_def_invalid_id1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_id_1.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("NoNE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_invalid_id_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid identifier (2).
 */
mu_test_begin(test_vartype_parser_def_invalid_id2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_id_2.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("NON%", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_invalid_id_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid definition due to missing '='.
 */
mu_test_begin(test_vartype_parser_def_invalid_eq)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_eq.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid definition due to an open-ended
 * regular expression.
 */
mu_test_begin(test_vartype_parser_def_invalid_regex)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_regex.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid definition due to a missing ':'.
 */
mu_test_begin(test_vartype_parser_def_invalid_colon)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_colon.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid definition due to an open-ended
 * error message.
 */
mu_test_begin(test_vartype_parser_def_invalid_errmsg)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_def_invalid_errmsg.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_invalid_errmsg.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_vartype_parser_def_comment1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_comment_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_comment_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition and comments around it.
 */
mu_test_begin(test_vartype_parser_def_comment2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_comment_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_comment_2.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a multi-line commented single definition,
 * comments above and below, and LF line endings.
 */
mu_test_begin(test_vartype_parser_def_comment3a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_comment_3a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_comment_3a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a multi-line commented single definition,
 * comments above and below, and CR/LF line endings.
 */
mu_test_begin(test_vartype_parser_def_comment3b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_def_comment_3b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_def_comment_3b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and non-escaped multi-line strings.
 */
mu_test_begin(test_vartype_parser_string_multiline1a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_1a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_1a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and non-escaped multi-line strings using CR/LF line breaks.
 */
mu_test_begin(test_vartype_parser_string_multiline1b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_1b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_1b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and multi-line strings using line continuation.
 */
mu_test_begin(test_vartype_parser_string_multiline2a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_2a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(".   *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected     anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_2a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and multi-line strings using line continuation and CR/LF line
 * breaks.
 */
mu_test_begin(test_vartype_parser_string_multiline2b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_2b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(".   *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected     anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_2b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and non-escaped multi-line strings.
 */
mu_test_begin(test_vartype_parser_string_multiline3a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_3a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_3a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and non-escaped multi-line strings using CR/LF line breaks.
 */
mu_test_begin(test_vartype_parser_string_multiline3b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_3b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_3b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and multi-line strings using line continuation.
 */
mu_test_begin(test_vartype_parser_string_multiline4a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_4a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".\\ *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected\\ anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_4a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines and multi-line strings using line continuation and CR/LF line
 * breaks.
 */
mu_test_begin(test_vartype_parser_string_multiline4b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_4b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".\\ *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected\\ anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_4b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_parser_string_multiline5a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_5a.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_5a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings
 */
mu_test_begin(test_vartype_parser_string_multiline5b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_5b.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_5b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_parser_string_multiline6a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_6a.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_6a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a single-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings
 */
mu_test_begin(test_vartype_parser_string_multiline6b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_6b.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_6b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, double-quoted, multi-line strings containing empty lines, and LF
 * line endings.
 */
mu_test_begin(test_vartype_parser_string_multiline7a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_7a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_7a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, double-quoted, multi-line strings containing empty lines, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_parser_string_multiline7b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_7b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_7b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, single-quoted, multi-line strings containing empty lines, and LF
 * line endings.
 */
mu_test_begin(test_vartype_parser_string_multiline8a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_8a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_8a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, single-quoted, multi-line strings containing empty lines, and
 * CR/LF line endings.
 */
mu_test_begin(test_vartype_parser_string_multiline8b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_8b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_8b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_parser_string_multiline9a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_9a.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_9a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings
 */
mu_test_begin(test_vartype_parser_string_multiline9b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_9b.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_9b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and LF
 * line endings
 */
mu_test_begin(test_vartype_parser_string_multiline10a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_10a.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_10a.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented definition distributed across
 * many lines, a double-quoted, incorrectly continued multi-line string, and
 * CR/LF line endings
 */
mu_test_begin(test_vartype_parser_string_multiline10b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_multiline_10b.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(". *", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_multiline_10b.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (1).
 */
mu_test_begin(test_vartype_parser_string_open1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_1.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (2).
 */
mu_test_begin(test_vartype_parser_string_open2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_2.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (3).
 */
mu_test_begin(test_vartype_parser_string_open3)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_3.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("\" ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (4).
 */
mu_test_begin(test_vartype_parser_string_open4)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_4.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_4.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (5).
 */
mu_test_begin(test_vartype_parser_string_open5)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_5.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (6).
 */
mu_test_begin(test_vartype_parser_string_open6)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_6.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an incomplete string (7).
 */
mu_test_begin(test_vartype_parser_string_open7)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/vartype_string_open_7.txt", &exit_code);

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(!def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything ", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_open_7.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (1).
 */
mu_test_begin(test_vartype_parser_string_escape1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str("\\ab", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("\\ab expected", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (2).
 */
mu_test_begin(test_vartype_parser_string_escape2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str("a\\b", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("a\\b expected", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (3).
 */
mu_test_begin(test_vartype_parser_string_escape3)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str("a\\", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected a\\", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (4).
 */
mu_test_begin(test_vartype_parser_string_escape4)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_4.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("\\ab", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("\\ab expected", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_4.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (5).
 */
mu_test_begin(test_vartype_parser_string_escape5)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_5.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("a\\b", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("a\\b expected", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (6).
 */
mu_test_begin(test_vartype_parser_string_escape6)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_6.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("a\\", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a\\", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing strings with escape characters (7).
 */
mu_test_begin(test_vartype_parser_string_escape7)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_string_escape_7.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("\\\"abc\"''\"", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("", def->errmsg->value);
    mu_assert_eq_str("", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_string_escape_7.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition with a BOM.
 */
mu_test_begin(test_vartype_parser_bom)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_bom.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_bom.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition with TABs.
 */
mu_test_begin(test_vartype_parser_tabs)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_tabs.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected\tanything", def->errmsg->value);
    mu_assert_eq_str("definition \t\tof NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_tabs.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a simple extension only.
 */
mu_test_begin(test_vartype_parser_ext1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(!ext->cond);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a simple commented extension.
 */
mu_test_begin(test_vartype_parser_ext2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(!ext->cond);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a simple
 * condition.
 */
mu_test_begin(test_vartype_parser_ext3)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_variable_to_expr(expr_variable_create_by_name(
        var_get_global_scope(),
        "OPT_XY"
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_3.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing a regular expression.
 */
mu_test_begin(test_vartype_parser_ext4a)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_4a.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "abc\\"))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_4a.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing a regular expression with escaped characters.
 */
mu_test_begin(test_vartype_parser_ext4b)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_4b.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a\"b'\\\\c\"d"))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_4b.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing a logical NOT.
 */
mu_test_begin(test_vartype_parser_ext5)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_5.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_logical_not_to_expr(expr_logical_not_create(
        var_get_global_scope(),
        expr_logical_not_to_expr(expr_logical_not_create(
            var_get_global_scope(),
            expr_variable_to_expr(expr_variable_create_by_name(
                var_get_global_scope(),
                "OPT_XY"
            ))
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_5.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing a logical AND.
 */
mu_test_begin(test_vartype_parser_ext6)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_6.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_logical_and_to_expr(expr_logical_and_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_YZ"
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_6.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing a logical OR.
 */
mu_test_begin(test_vartype_parser_ext7)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_7.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_logical_or_to_expr(expr_logical_or_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_YZ"
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_7.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing an equality operator.
 */
mu_test_begin(test_vartype_parser_ext8)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_8.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_8.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a condition
 * containing an inequality operator.
 */
mu_test_begin(test_vartype_parser_ext9)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_9.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_unequal_to_expr(expr_unequal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_9.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented extension with a complex
 * condition.
 */
mu_test_begin(test_vartype_parser_ext10)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_10.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
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
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_10.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing an invalid quoted string.
 */
mu_test_begin(test_vartype_parser_ext11)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_ext_11.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(!ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(!ext->cond->valid);
    expected = expr_equal_to_expr(expr_equal_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(
            var_get_global_scope(),
            "abc"
        ))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_ext_11.txt' (package test) 1:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing multiple definitions.
 */
mu_test_begin(test_vartype_parser_multi1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_multi_1.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NUMERIC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a number", def->errmsg->value);
    mu_assert_eq_str("definition of NUMERIC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing multiple definitions with comments
 * interspersed.
 */
mu_test_begin(test_vartype_parser_multi2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_multi_2.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_2.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NUMERIC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a number", def->errmsg->value);
    mu_assert_eq_str("definition of NUMERIC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_2.txt' (package test) 4:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing multiple definitions and extensions with
 * comments interspersed.
 */
mu_test_begin(test_vartype_parser_multi3)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;
    struct expr_t *expected;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_multi_3.txt", &exit_code
    );

    mu_assert(tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_3.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NUMERIC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a number", def->errmsg->value);
    mu_assert_eq_str("definition of NUMERIC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_3.txt' (package test) 4:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(ext);
    mu_assert(ext->valid);
    mu_assert(ext->id->valid);
    mu_assert_eq_str("NONE", ext->id->name);
    mu_assert(ext->cond);
    mu_assert(ext->cond->valid);
    expected = expr_match_to_expr(expr_match_create(
        var_get_global_scope(),
        expr_variable_to_expr(expr_variable_create_by_name(
            var_get_global_scope(),
            "OPT_XY"
        )),
        expr_literal_to_expr(expr_literal_create_string(var_get_global_scope(), "a\"b'\\\\c\"d"))
    ));
    mu_assert(expr_equal(expected, ext->cond->expr));
    expr_destroy(expected);
    mu_assert(ext->regex->valid);
    mu_assert_eq_int('\'', ext->regex->delim);
    mu_assert_eq_str(".*", ext->regex->value);
    mu_assert(ext->errmsg->valid);
    mu_assert_eq_int('\'', ext->errmsg->delim);
    mu_assert_eq_str("expected anything", ext->errmsg->value);
    mu_assert_eq_str("extension of NONE", ext->comment);
    locstr = location_toString(ext->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_3.txt' (package test) 6:0]", locstr);
    free(locstr);

    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing multiple definitions of which the one in the
 * middle is syntactically incorrect (1).
 */
mu_test_begin(test_vartype_parser_multi_wrong1)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_multi_wrong_1.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_wrong_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(!def->valid);
    mu_assert(!def->id->valid);
    mu_assert_eq_str("AbC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str("AbC", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected AbC", def->errmsg->value);
    mu_assert_eq_str("definition of AbC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_wrong_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NUMERIC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a number", def->errmsg->value);
    mu_assert_eq_str("definition of NUMERIC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_wrong_1.txt' (package test) 3:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()


/**
 * Tests parsing file containing multiple definitions of which the one in the
 * middle is syntactically incorrect (2).
 */
mu_test_begin(test_vartype_parser_multi_wrong2)
    struct vartype_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it_def, def, struct vartype_def_t);
    DECLARE_ARRAY_ITER(it_ext, ext, struct vartype_ext_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser(
        "grammar/tests/input/vartype_multi_wrong_2.txt", &exit_code
    );

    mu_assert(!tree->ok);

    init_array_iterator(&it_def, tree->definitions);
    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NONE", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('\'', def->regex->delim);
    mu_assert_eq_str(".*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('\'', def->errmsg->delim);
    mu_assert_eq_str("expected anything", def->errmsg->value);
    mu_assert_eq_str("definition of NONE", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_wrong_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(def);
    mu_assert(def->valid);
    mu_assert(def->id->valid);
    mu_assert_eq_str("NUMERIC", def->id->name);
    mu_assert(def->regex->valid);
    mu_assert_eq_int('"', def->regex->delim);
    mu_assert_eq_str("0|[1-9][0-9]*", def->regex->value);
    mu_assert(def->errmsg->valid);
    mu_assert_eq_int('"', def->errmsg->delim);
    mu_assert_eq_str("expected a number", def->errmsg->value);
    mu_assert_eq_str("definition of NUMERIC", def->comment);
    locstr = location_toString(def->location);
    mu_assert_eq_str("[variable type file 'grammar/tests/input/vartype_multi_wrong_2.txt' (package test) 3:0]", locstr);
    free(locstr);

    def = (struct vartype_def_t *) get_next_elem(&it_def);
    mu_assert(!def);

    init_array_iterator(&it_ext, tree->extensions);
    ext = (struct vartype_ext_t *) get_next_elem(&it_ext);
    mu_assert(!ext);

    vartype_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

void
test_vartype_parser(void)
{
    mu_run_test(test_vartype_parser_empty_file);
    mu_run_test(test_vartype_parser_comment1);
    mu_run_test(test_vartype_parser_comment2);
    mu_run_test(test_vartype_parser_def1);
    mu_run_test(test_vartype_parser_def2);
    mu_run_test(test_vartype_parser_def3);
    mu_run_test(test_vartype_parser_def4);
    mu_run_test(test_vartype_parser_def_invalid_id1);
    mu_run_test(test_vartype_parser_def_invalid_id2);
    mu_run_test(test_vartype_parser_def_invalid_eq);
    mu_run_test(test_vartype_parser_def_invalid_regex);
    mu_run_test(test_vartype_parser_def_invalid_colon);
    mu_run_test(test_vartype_parser_def_invalid_errmsg);
    mu_run_test(test_vartype_parser_def_comment1);
    mu_run_test(test_vartype_parser_def_comment2);
    mu_run_test(test_vartype_parser_def_comment3a);
    mu_run_test(test_vartype_parser_def_comment3b);
    mu_run_test(test_vartype_parser_string_multiline1a);
    mu_run_test(test_vartype_parser_string_multiline1b);
    mu_run_test(test_vartype_parser_string_multiline2a);
    mu_run_test(test_vartype_parser_string_multiline2b);
    mu_run_test(test_vartype_parser_string_multiline3a);
    mu_run_test(test_vartype_parser_string_multiline3b);
    mu_run_test(test_vartype_parser_string_multiline4a);
    mu_run_test(test_vartype_parser_string_multiline4b);
    mu_run_test(test_vartype_parser_string_multiline5a);
    mu_run_test(test_vartype_parser_string_multiline5b);
    mu_run_test(test_vartype_parser_string_multiline6a);
    mu_run_test(test_vartype_parser_string_multiline6b);
    mu_run_test(test_vartype_parser_string_multiline7a);
    mu_run_test(test_vartype_parser_string_multiline7b);
    mu_run_test(test_vartype_parser_string_multiline8a);
    mu_run_test(test_vartype_parser_string_multiline8b);
    mu_run_test(test_vartype_parser_string_multiline9a);
    mu_run_test(test_vartype_parser_string_multiline9b);
    mu_run_test(test_vartype_parser_string_multiline10a);
    mu_run_test(test_vartype_parser_string_multiline10b);
    mu_run_test(test_vartype_parser_string_open1);
    mu_run_test(test_vartype_parser_string_open2);
    mu_run_test(test_vartype_parser_string_open3);
    mu_run_test(test_vartype_parser_string_open4);
    mu_run_test(test_vartype_parser_string_open5);
    mu_run_test(test_vartype_parser_string_open6);
    mu_run_test(test_vartype_parser_string_open7);
    mu_run_test(test_vartype_parser_string_escape1);
    mu_run_test(test_vartype_parser_string_escape2);
    mu_run_test(test_vartype_parser_string_escape3);
    mu_run_test(test_vartype_parser_string_escape4);
    mu_run_test(test_vartype_parser_string_escape5);
    mu_run_test(test_vartype_parser_string_escape6);
    mu_run_test(test_vartype_parser_string_escape7);
    mu_run_test(test_vartype_parser_bom);
    mu_run_test(test_vartype_parser_tabs);
    mu_run_test(test_vartype_parser_ext1);
    mu_run_test(test_vartype_parser_ext2);
    mu_run_test(test_vartype_parser_ext3);
    mu_run_test(test_vartype_parser_ext4a);
    mu_run_test(test_vartype_parser_ext4b);
    mu_run_test(test_vartype_parser_ext5);
    mu_run_test(test_vartype_parser_ext6);
    mu_run_test(test_vartype_parser_ext7);
    mu_run_test(test_vartype_parser_ext8);
    mu_run_test(test_vartype_parser_ext9);
    mu_run_test(test_vartype_parser_ext10);
    mu_run_test(test_vartype_parser_ext11);
    mu_run_test(test_vartype_parser_multi1);
    mu_run_test(test_vartype_parser_multi2);
    mu_run_test(test_vartype_parser_multi3);
    mu_run_test(test_vartype_parser_multi_wrong1);
    mu_run_test(test_vartype_parser_multi_wrong2);
}
