/*----------------------------------------------------------------------------
 *  regex.c - functions for processing regular expressions
 *
 *  Copyright (c) 2002-2005 - Jean Wolter
 *  Copyright (c) 2005-2016 - fli4l-Team <team@fli4l.de>
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
 *  Creation:    2002-03-01  jw5
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>
#include <stdarg.h>
#include <ctype.h>

#include <dirent.h>

#include "regex.h"
#include "log.h"
#include "grammar/vartype.h"
#include "grammar/vartype_parser.h"
#include "grammar/vartype_scanner.h"
#include "grammar/scanner.h"
#include "package.h"
#include "vartype.h"
#include "var.h"
#include "expression/expr.h"
#include "expression/expr_literal.h"
#include "expression/expr_variable.h"
#include "expression/expr_match.h"

/* #define DEBUG_REGEX */

static struct vartype_parse_tree_t *tree;

void
regexp_init(void)
{
    tree = vartype_parse_tree_create();
}

void
regexp_done(void)
{
    vartype_parse_tree_destroy(tree);
}

/*----------------------------------------------------------------------------
 * read file with regular expressions
 *----------------------------------------------------------------------------
 */

void
regexp_read_file(struct package_file_t *file, struct package_t *package)
{
    char const *expr_file = package_file_get_name(file);
    FILE *f = fopen(expr_file, "r");
    if (f) {
        yyscan_t scanner;
        int parse_result;

        if (vartype_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", expr_file);
            fclose(f);
            tree->ok = FALSE;
        }
        else {
            vartype_set_in(f, scanner);
            parse_result = vartype_parse(scanner, file, tree);
            vartype_lex_destroy(scanner);
            fclose(f);

            if (parse_result != 0) {
                tree->ok = FALSE;
            }
        }
    }
    else {
        log_error("Error opening variable types file %s\n", expr_file);
        tree->ok = FALSE;
    }

    (void) package;
}

BOOL
regexp_process_definitions(void)
{
    BOOL result = TRUE;

    ARRAY_ITER(it, tree->definitions, def, struct vartype_def_t) {
        struct location_t *loc = location_clone(def->location);
        if (!vartype_add(def->id->name, def->regex->value, def->errmsg->value, def->comment, loc)) {
            char *locstr = location_toString(def->location);
            log_error(
                "Adding definition of variable type '%s' at %s failed.\n",
                def->id->name,
                locstr
            );
            free(locstr);
            location_destroy(loc);
            result = FALSE;
        }
    }
    return result && tree->ok;
}

BOOL
regexp_process_extensions(void)
{
    BOOL result = TRUE;

    ARRAY_ITER(it, tree->extensions, ext, struct vartype_ext_t) {
        struct vartype_t *vartype = vartype_try_get(ext->id->name);
        if (!vartype) {
            log_info(LOG_REGEXP,
                     "Tried to extend undefined regular expression '%s'.\n",
                      ext->id->name);
        }
        else {
            struct expr_t *condition;
            struct location_t *loc;

            if (ext->cond) {
                condition = expr_clone(ext->cond->expr);
            }
            else {
                condition = NULL;
            }

            loc = location_clone(ext->location);
            if (!vartype_extend(vartype, ext->regex->value, condition, ext->errmsg->value, ext->comment, loc)) {
                char *locstr = location_toString(ext->location);
                log_error(
                    "Extension of variable type '%s' at %s failed.\n",
                    ext->id->name,
                    locstr
                );
                free(locstr);
                location_destroy(loc);
                if (condition != NULL) {
                    expr_destroy(condition);
                }
                result = FALSE;
            }
        }
    }
    return result;
}

int regexp_user(char const *value, char const *expr, size_t nmatch, regmatch_t * pmatch,
                BOOL modify_expr, char const *file, int line)
{
    struct vartype_t *type;

    log_info(LOG_REGEXP, "checking '%s' against regexp '%s'\n", value, expr);

    type = vartype_add_temporary(expr, location_create_internal(PACKAGE_FILE_TYPE_EXTCHECK), modify_expr);
    if (!type) {
        return REGEX_COMPILE_ERROR;
    }
    else {
        regmatch_t *matches = vartype_match_ext(type, value);
        if (matches) {
            regmatch_t *match_ptr = matches;
            while (match_ptr->rm_so != -1 && nmatch > 0) {
                *pmatch = *match_ptr;
                ++pmatch;
                ++match_ptr;
                --nmatch;
            }
            while (nmatch > 0) {
                pmatch->rm_so = pmatch->rm_eo = -1;
                ++pmatch;
                --nmatch;
            }
            free(matches);
            vartype_remove(type);
            return REGEX_MATCHED;
        }
        else {
            vartype_remove(type);
            return REGEX_NOT_MATCHED;
        }
    }
    (void) file;
    (void) line;
}

int
regexp_exists(char const *name)
{
    return vartype_try_get(name) != NULL;
}

int
regexp_chkvar(char const *name, char const *value,
              char const *regexp_name, char const *user_regexp)
{
    int res;
    char const *err_msg = NULL;

    if (!regexp_name) {
        err_msg = "user supplied regular expression";
        res = regexp_user(value, user_regexp, 0, NULL, TRUE, __FILE__, __LINE__);
    }
    else {
        struct vartype_t *p = vartype_try_get(regexp_name);
        if (!p) {
            log_error("unknown regular expression %s\n", regexp_name);
            return ERR;
        }

        if (!vartype_match(p, value)) {
            log_info(LOG_VERBOSE, "regex error for value '%s' "
                     "and regexp '%s'\n",
                     value, vartype_get_regex(p, FALSE));
            err_msg = vartype_get_error_message(p);
            res = REGEX_NOT_MATCHED;
        }
        else {
            res = REGEX_MATCHED;
        }
    }

    if (res == REGEX_MATCHED) {
        return OK;
    }
    else if (res == REGEX_NOT_MATCHED) {
        char *location = get_variable_src(name);
        log_error("%s wrong value of variable %s: '%s' (%s)\n",
                  location, name, value, err_msg);
        free(location);
    }

    return ERR;
}

int
regexp_find_type(char const *regexp_name)
{
    struct vartype_t *p = vartype_try_get(regexp_name);
    return p ? (vartype_get_expression_type(p) == EXPR_TYPE_INTEGER ? TYPE_NUMERIC : TYPE_UNKNOWN) : -1;
}
