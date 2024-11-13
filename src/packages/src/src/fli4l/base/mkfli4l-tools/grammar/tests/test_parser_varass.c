/*****************************************************************************
 *  @file grammar/tests/test_parser_varass.c
 *  Functions for testing the parsing of variable assignment files.
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
#include "var.h"

#define MU_TESTSUITE "grammar.varass.parser"

#define TEST_PACKAGE "test"

/**
 * Runs the scanner and parser for a given input file.
 *
 * @param name
 *  The name of the input file.
 * @param priority
 *  The access priority to use.
 * @param exit_code
 *  Receives the exit code of the parser.
 * @return
 *  A parse tree.
 */
static struct varass_parse_tree_t *
run_scanner_and_parser(char const *name, int priority, int *exit_code)
{
    struct package_t *package = package_get_or_add(TEST_PACKAGE);
    struct package_file_t *file
        = package_set_file(package, PACKAGE_FILE_TYPE_CONFIG, name);
    struct varass_parse_tree_t *tree = varass_parse_tree_create();

    FILE *f = fopen(name, "r");
    if (f) {
        yyscan_t scanner;
        if (varass_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing parser for %s\n", name);
        }
        else {
            varass_set_in(f, scanner);
            *exit_code = varass_parse(scanner, file, tree, priority);
            varass_lex_destroy(scanner);
        }
        fclose(f);
    }
    else {
        log_error("Error opening variable assignment file %s\n", name);
    }

    return tree;
}

/**
 * Tests parsing empty file.
 */
mu_test_begin(test_varass_parser_empty_file)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/empty.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (1).
 */
mu_test_begin(test_varass_parser_comment1)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/comment_1.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing only comments (2).
 */
mu_test_begin(test_varass_parser_comment2)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/comment_2.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing uncommented assignments.
 */
mu_test_begin(test_varass_parser_ass1)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_1.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc\\\"\\", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_1.txt' (package test) 1:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR2", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('"', ass->value->delim);
    mu_assert_eq_str("abc\\\"'", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented assignment.
 */
mu_test_begin(test_varass_parser_ass2)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_2.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc", ass->value->value);
    mu_assert_eq_str("setting VAR1", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_2.txt' (package test) 1:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented assignment distributed across
 * many lines.
 */
mu_test_begin(test_varass_parser_ass3)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_3.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc", ass->value->value);
    mu_assert_eq_str("setting VAR1", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_3.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a commented single definition and comments
 * above and below.
 */
mu_test_begin(test_varass_parser_ass_comment1)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_comment_1.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc", ass->value->value);
    mu_assert_eq_str("setting VAR1", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_comment_1.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing a single definition and comments around it.
 */
mu_test_begin(test_varass_parser_ass_comment2)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_comment_2.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_comment_2.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing assignments to structure members (new syntax).
 */
mu_test_begin(test_varass_parser_ass_new_syntax1)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_new_syntax1.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("B", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("A", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("x", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_new_syntax1.txt' (package test) 1:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("C", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("B", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("A", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("y", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_new_syntax1.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing assignments to array slots (new syntax).
 */
mu_test_begin(test_varass_parser_ass_new_syntax2)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_new_syntax2.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_ABSOLUTE, target->u.array_member.index->kind);
    mu_assert_eq_int(1, target->u.array_member.index->u.absolute.number);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("VAR", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc\\\"\\", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_new_syntax2.txt' (package test) 1:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_ABSOLUTE, target->u.array_member.index->kind);
    mu_assert_eq_int(33, target->u.array_member.index->u.absolute.number);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("XY", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_ABSOLUTE, target->u.array_member.index->kind);
    mu_assert_eq_int(2, target->u.array_member.index->u.absolute.number);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("VAR", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('"', ass->value->delim);
    mu_assert_eq_str("abc\\\"'", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_new_syntax2.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("Y", target->u.struct_member.member->name);
    target = target->u.struct_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_ABSOLUTE, target->u.array_member.index->kind);
    mu_assert_eq_int(3, target->u.array_member.index->u.absolute.number);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("X", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("VAR", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('"', ass->value->delim);
    mu_assert_eq_str("z", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_new_syntax2.txt' (package test) 3:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing structured assignments (1).
 */
mu_test_begin(test_varass_parser_ass_structured1)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;
    struct varass_list_node_t *node;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_structured1.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("IP_NET", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("192.168.1.0/24", ass->value->value);
    mu_assert_eq_str("my LAN", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured1.txt' (package test) 1:0]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node = ass->nested_assignments->first;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("DEV", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("eth0", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured1.txt' (package test) 3:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("NAME", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("LAN", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured1.txt' (package test) 4:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("COMMENT", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("bla", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured1.txt' (package test) 5:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(!node);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing structured assignments (2).
 */
mu_test_begin(test_varass_parser_ass_structured2)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;
    struct varass_list_node_t *node;
    struct varass_list_node_t *node1;
    struct varass_list_node_t *node2;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_structured2.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("CIRC", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("my WAN", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 1:0]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node = ass->nested_assignments->first;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("NAME", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("DSL", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 3:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("TYPE", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("ppp", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 4:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("ENABLED", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("yes", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 5:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("UP", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("yes", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 6:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("NETS_IPV4", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("0.0.0.0/0", ass->value->value);
    mu_assert_eq_str("default route", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 7:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("CLASS", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("Internet", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 8:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("PPP", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 9:8]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node1 = ass->nested_assignments->first;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("TYPE", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("ethernet", ass->value->value);
    mu_assert_eq_str("PPPoE", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 10:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("USERID", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("anonymer", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 11:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("PASSWORD", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("surfer", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 12:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("ETHERNET", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("PPPoE specific settings", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 13:16]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node2 = ass->nested_assignments->first;
    mu_assert(node2);
    ass = node2->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("DEV", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("eth1", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 15:24]", locstr);
    free(locstr);

    node2 = node2->next;
    mu_assert(node2);
    ass = node2->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_STRUCTURE_MEMBER, target->kind);
    mu_assert(target->u.struct_member.member->valid);
    mu_assert_eq_str("TYPE", target->u.struct_member.member->name);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("kernel", ass->value->value);
    mu_assert_eq_str("more performant", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured2.txt' (package test) 16:24]", locstr);
    free(locstr);

    node2 = node2->next;
    mu_assert(!node2);

    node1 = node1->next;
    mu_assert(!node1);

    node = node->next;
    mu_assert(!node);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing structured assignments (3).
 */
mu_test_begin(test_varass_parser_ass_structured3)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;
    struct varass_list_node_t *node;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_structured3.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("MULTI", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("first row", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 1:0]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node = ass->nested_assignments->first;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("1", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 3:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("2", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 4:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(!node);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("MULTI", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("second row", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 6:0]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node = ass->nested_assignments->first;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("3", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 8:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("4", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured3.txt' (package test) 9:8]", locstr);
    free(locstr);

    node = node->next;
    mu_assert(!node);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing structured assignments (4).
 */
mu_test_begin(test_varass_parser_ass_structured4)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;
    struct varass_target_t *target;
    struct varass_list_node_t *node;
    struct varass_list_node_t *node1;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_ass_structured4.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(target->u.top_level.id->valid);
    mu_assert_eq_str("MULTI", target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 1:0]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node = ass->nested_assignments->first;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("first row", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 3:8]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node1 = ass->nested_assignments->first;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("1", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 5:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("2", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 6:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(!node1);

    node = node->next;
    mu_assert(node);
    ass = node->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(!ass->value);
    mu_assert_eq_str("second row", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 8:8]", locstr);
    free(locstr);

    mu_assert(ass->nested_assignments);
    node1 = ass->nested_assignments->first;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("3", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 10:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(node1);
    ass = node1->assignment;
    mu_assert(ass);
    mu_assert(ass->valid);
    target = ass->target;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_ARRAY_MEMBER, target->kind);
    mu_assert_eq_int(VARASS_INDEX_NEXT_FREE, target->u.array_member.index->kind);
    target = target->u.array_member.parent;
    mu_assert(target);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, target->kind);
    mu_assert(!target->u.top_level.id);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("4", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_ass_structured4.txt' (package test) 11:16]", locstr);
    free(locstr);

    node1 = node1->next;
    mu_assert(!node1);

    node = node->next;
    mu_assert(!node);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

/**
 * Tests parsing file containing assignments with a BOM.
 */
mu_test_begin(test_varass_parser_bom)
    struct varass_parse_tree_t *tree;
    int exit_code;
    DECLARE_ARRAY_ITER(it, ass, struct varass_t);
    char *locstr;

    package_init_module();
    tree = run_scanner_and_parser("grammar/tests/input/varass_bom.txt",
            VAR_PRIORITY_CONFIGURATION, &exit_code);

    mu_assert(tree->ok);

    init_array_iterator(&it, tree->assignments);
    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR1", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('\'', ass->value->delim);
    mu_assert_eq_str("abc\\\"\\", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_bom.txt' (package test) 1:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(ass);
    mu_assert(ass->valid);
    mu_assert_eq_int(VARASS_TARGET_TOP_LEVEL, ass->target->kind);
    mu_assert(ass->target->u.top_level.id->valid);
    mu_assert_eq_str("VAR2", ass->target->u.top_level.id->name);
    mu_assert_eq_int(VAR_PRIORITY_CONFIGURATION, ass->priority);
    mu_assert(ass->value->valid);
    mu_assert_eq_int('"', ass->value->delim);
    mu_assert_eq_str("abc\\\"'", ass->value->value);
    mu_assert_eq_str("", ass->comment);
    locstr = location_toString(ass->location);
    mu_assert_eq_str("[configuration file 'grammar/tests/input/varass_bom.txt' (package test) 2:0]", locstr);
    free(locstr);

    ass = (struct varass_t *) get_next_elem(&it);
    mu_assert(!ass);

    varass_parse_tree_destroy(tree);
    package_fini_module();
mu_test_end()

void
test_varass_parser(void)
{
    mu_run_test(test_varass_parser_empty_file);
    mu_run_test(test_varass_parser_comment1);
    mu_run_test(test_varass_parser_comment2);
    mu_run_test(test_varass_parser_ass1);
    mu_run_test(test_varass_parser_ass2);
    mu_run_test(test_varass_parser_ass3);
    mu_run_test(test_varass_parser_ass_comment1);
    mu_run_test(test_varass_parser_ass_comment2);
    mu_run_test(test_varass_parser_ass_new_syntax1);
    mu_run_test(test_varass_parser_ass_new_syntax2);
    mu_run_test(test_varass_parser_ass_structured1);
    mu_run_test(test_varass_parser_ass_structured2);
    mu_run_test(test_varass_parser_ass_structured3);
    mu_run_test(test_varass_parser_ass_structured4);
    mu_run_test(test_varass_parser_bom);
}
