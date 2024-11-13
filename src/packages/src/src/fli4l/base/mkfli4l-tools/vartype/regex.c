/*****************************************************************************
 *  @file vartype/regex.c
 *  Functions for processing regular expressions associated with variable
 *  types.
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

#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "core.h"
#include "regex.h"
#include "extension.h"

#define MODULE "vartype::regex"

/**
 * Prefix for complete regular expressions.
 */
#define VARTYPE_COMPLETE_PREFIX "^("
/**
 * Suffix for complete regular expressions.
 */
#define VARTYPE_COMPLETE_SUFFIX ")$"

BOOL
vartype_compile(struct vartype_t *THIS)
{
    char *regex_str;
    char *complete_regex;
    int comp_result;

    if (THIS->compiled_state) {
        return TRUE;
    }

    regex_str = vartype_get_regex(THIS, TRUE);
    if (!regex_str) {
        char *locstr = location_toString(THIS->location);
        regex_str = vartype_get_regex(THIS, FALSE);
        log_error(
            MODULE ": Compiling regular expression '%s' of variable type '%s' at %s failed.\n",
            regex_str,
            THIS->name ? THIS->name : VARTYPE_NAME_ANONYMOUS,
            locstr
        );
        free(regex_str);
        free(locstr);
        return FALSE;
    }

    complete_regex = (char *) safe_malloc(
        strlen(regex_str) +
        sizeof(VARTYPE_COMPLETE_PREFIX) - 1 +
        sizeof(VARTYPE_COMPLETE_SUFFIX) - 1 +
        1
    );
    complete_regex[0] = '\0';
    if (THIS->incomplete) {
        strcat(complete_regex, VARTYPE_COMPLETE_PREFIX);
    }
    strcat(complete_regex, regex_str);
    if (THIS->incomplete) {
        strcat(complete_regex, VARTYPE_COMPLETE_SUFFIX);
    }

    free(regex_str);

    THIS->compiled_state = (struct vartype_compiled_state_t *)
        safe_malloc(sizeof(struct vartype_compiled_state_t));
    THIS->compiled_state->complete_error_msg
        = vartype_extension_assemble_error_message(THIS->extensions, THIS);
    THIS->compiled_state->complete_comment
        = vartype_extension_assemble_comment(THIS->extensions, THIS);

    comp_result = regcomp(&THIS->compiled_state->regex,
                            complete_regex, REG_EXTENDED | REG_NEWLINE);
    if (comp_result != 0) {
        char *const locstr = location_toString(THIS->location);
        size_t const errbuf_size
            = regerror(comp_result, &THIS->compiled_state->regex, NULL, 0);
        char *const errbuf = (char *) safe_malloc(errbuf_size);
        regerror(comp_result, &THIS->compiled_state->regex, errbuf, errbuf_size);

        regex_str = vartype_get_regex(THIS, FALSE);
        log_error(
            MODULE ": Compiling regular expression '%s' (expanded '%s') of variable type '%s' at %s failed: %s.\n",
            regex_str,
            complete_regex,
            THIS->name ? THIS->name : VARTYPE_NAME_ANONYMOUS,
            locstr,
            errbuf
        );

        free(regex_str);
        free(errbuf);
        free(locstr);
        vartype_free_compiled_state(THIS);
        free(complete_regex);
        return FALSE;
    }
    else {
        char *const locstr = location_toString(THIS->location);
        regex_str = vartype_get_regex(THIS, FALSE);
        log_info(
            LOG_EXP,
            MODULE ": Compiling regular expression '%s' (expanded '%s') of variable type '%s' at %s succeeded.\n",
            regex_str,
            complete_regex,
            THIS->name ? THIS->name : VARTYPE_NAME_ANONYMOUS,
            locstr
        );

        free(locstr);
        free(regex_str);
        free(complete_regex);
        return TRUE;
    }
}

void
vartype_free_compiled_state(struct vartype_t *THIS)
{
    regfree(&THIS->compiled_state->regex);
    free(THIS->compiled_state->complete_comment);
    free(THIS->compiled_state->complete_error_msg);
    free(THIS->compiled_state);
    THIS->compiled_state = NULL;
}

BOOL
vartype_match(struct vartype_t *THIS, char const *value)
{
    int result;

    my_assert(THIS->compiled_state);

    result = regexec(&THIS->compiled_state->regex, value, 0, NULL, 0);
    if (result == 0) {
        return TRUE;
    }
    else {
        char *const locstr = location_toString(THIS->location);
        char *regex = vartype_get_regex(THIS, TRUE);
        log_info(
            LOG_REGEXP,
            MODULE ": Value '%s' does not match regular expression '%s' of type '%s' at %s: %s.\n",
            value,
            regex,
            THIS->name ? THIS->name : VARTYPE_NAME_ANONYMOUS,
            locstr,
            THIS->compiled_state->complete_error_msg
        );
        free(regex);
        free(locstr);
        return FALSE;
    }
}

regmatch_t *
vartype_match_ext(struct vartype_t *THIS, char const *value)
{
    int num_sub;
    regmatch_t *matches;
    int result;

    my_assert(THIS->compiled_state);

    num_sub = THIS->compiled_state->regex.re_nsub + 1;
    matches = (regmatch_t *) safe_malloc((num_sub + 1) * sizeof(regmatch_t));
    result = regexec(&THIS->compiled_state->regex, value, num_sub, matches, 0);
    if (result == 0) {
        matches[num_sub].rm_so = matches[num_sub].rm_eo = -1;
        return matches;
    }
    else {
        char *const locstr = location_toString(THIS->location);
        char *regex = vartype_get_regex(THIS, TRUE);
        log_info(
            LOG_REGEXP,
            MODULE ": Value '%s' does not match regular expression '%s' of type '%s' at %s: %s.\n",
            value,
            regex,
            THIS->name ? THIS->name : VARTYPE_NAME_ANONYMOUS,
            locstr,
            THIS->compiled_state->complete_error_msg
        );
        free(regex);
        free(locstr);
        free(matches);
        return NULL;
    }
}
