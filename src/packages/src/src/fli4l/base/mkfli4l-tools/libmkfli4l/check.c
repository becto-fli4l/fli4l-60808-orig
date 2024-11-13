/*----------------------------------------------------------------------------
 *  check.c - functions for checking variables
 *
 *  Copyright (c) 2001 - Frank Meyer
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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
 *  Creation:    2001-08-12  fm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "check.h"
#include "var.h"
#include "regex.h"
#include "log.h"
#include "parse.h"
#include "str.h"
#include "cfg.h"
#include "array.h"
#include "grammar/vardef.h"
#include "grammar/vardef_parser.h"
#include "grammar/vardef_scanner.h"
#include "package.h"
#include "vartype.h"
#include "var.h"
#include "expression.h"

#if 0
#define SUNDAY              0                       /* 1st day of unix week */
#define HOURS_PER_DAY       24                      /* hours per day        */
#define DAYS_PER_WEEK       7                       /* days per week        */

static int time_table[HOURS_PER_DAY * DAYS_PER_WEEK];  /* 24 * 7 hours */
static int local_time_table[HOURS_PER_DAY * DAYS_PER_WEEK];  /* 24 * 7 hours */
static char **time_variables;
static char **time_values;
#endif
static struct vardef_parse_tree_t *tree;

#if 0
static char const *en_wday_strings[DAYS_PER_WEEK] = {
    "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa",
};

static char const *de_wday_strings[DAYS_PER_WEEK] = {
    "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa",
};
#endif

BOOL
check_var_defined(char const *var)
{
    return var_try_get(var) != NULL;
}

BOOL
check_var_numeric(char const *var)
{
    struct var_t *v = var_try_get(var);
    return v &&
        var_get_type(v) == vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
}

BOOL
check_var_optional(char const *var)
{
    struct var_t *v = var_try_get(var);
    return v && !var_get_properties(v)->optional;
}

void check_init(void)
{
    tree = vardef_parse_tree_create();
}

void
read_check_file(struct package_file_t *file, struct package_t *package)
{
    char const *fname = package_file_get_name(file);
    FILE *f = fopen(fname, "r");
    if (f) {
        yyscan_t scanner;
        int parse_result;

        if (vardef_lex_init_extra(file, &scanner) != 0) {
            log_error("Error initializing scanner for %s\n", fname);
            fclose(f);
            tree->ok = FALSE;
        }
        else {
            vardef_set_in(f, scanner);
            parse_result = vardef_parse(scanner, file, tree);
            vardef_lex_destroy(scanner);
            fclose(f);

            if (parse_result != 0) {
                tree->ok = FALSE;
            }
        }
    }
    else {
        log_error("Error opening variable definitions file %s\n", fname);
        tree->ok = FALSE;
    }

    (void) package;
}

static BOOL
resolve_expressions(void *entry, void *data)
{
    struct var_t *var = (struct var_t *) entry;
    if (!var_has_valid_condition(var)) {
        /* error message has already been printed */
        *((BOOL *) data) = FALSE;
    }
    else if (!var_has_valid_default_value(var)) {
        /* error message has already been printed */
        *((BOOL *) data) = FALSE;
    }
    return TRUE;
    UNUSED(data);
}

static BOOL
resolve_template_expressions(void *entry, void *data)
{
    struct var_template_t *tmpl = (struct var_template_t *) entry;
    if (!var_template_has_valid_condition(tmpl)) {
        /* error message has already been printed */
        *((BOOL *) data) = FALSE;
    }
    else if (!var_template_has_valid_default_value(tmpl)) {
        /* error message has already been printed */
        *((BOOL *) data) = FALSE;
    }
    return TRUE;
    UNUSED(data);
}

BOOL
check_process_definitions(void)
{
    BOOL result = TRUE;
    struct var_scope_t *const scope = var_get_global_scope();

    ARRAY_ITER(it, tree->definitions, def, struct vardef_t) {
        struct location_t *loc;
        struct vartype_t *type;
        struct var_properties_t *props = var_properties_create();

        props->optional = def->optional;
        var_properties_set_extra(
            props,
            var_extra_create(create_var_flags(FALSE, NULL), clone_var_flags, free)
        );

        if (def->cond) {
            var_properties_set_condition(props, expr_clone(def->cond->expr));
        }

        if (def->defvalue) {
            var_properties_set_default_value(props, expr_clone(def->defvalue->expr));
        }

        if (def->comment) {
            var_properties_set_comment(props, def->comment);
        }

        if (def->type->named) {
            type = vartype_get(def->type->u.id->name);
            if (!type) {
                char *locstr = location_toString(def->location);
                log_error(
                    "Variable type '%s' referenced by the variable '%s' at %s does not exist.\n",
                    def->type->u.id->name,
                    def->id->name,
                    locstr
                );
                free(locstr);
                var_properties_destroy(props);
                result = FALSE;
                continue;
            }
            else if (type == vartype_get(VARTYPE_PREDEFINED_BOOLEAN)) {
                var_properties_set_disabled_value(props, VALUE_BOOLEAN_FALSE);
            }
            else if (type == vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER)) {
                var_properties_set_disabled_value(props, "0");
            }
        }
        else {
            type = vartype_add_temporary(def->type->u.regex, def->location, TRUE);
            if (!type) {
                char *locstr = location_toString(def->location);
                log_error(
                    "Variable type '%s' referenced by the variable '%s' at %s is invalid.\n",
                    def->type->u.regex,
                    def->id->name,
                    locstr
                );
                free(locstr);
                var_properties_destroy(props);
                result = FALSE;
                continue;
            }
        }

        loc = location_clone(def->location);

        if (!var_add_with_array_bound(
                scope, def->id->name,
                def->array_bound ? def->array_bound->name : NULL, type,
                VAR_PRIORITY_CONFIGURATION, loc, props)) {
            char *locstr = location_toString(def->location);
            log_error(
                "Adding definition of variable '%s' at %s failed.\n",
                def->id->name,
                locstr
            );
            free(locstr);
            location_destroy(loc);
            var_properties_destroy(props);
            result = FALSE;
        }
    }

    var_scope_foreach(var_get_global_scope(), TRUE, &resolve_expressions,
                      &result);

    return result && tree->ok;
}

BOOL
check_process_template_definitions(void)
{
    BOOL result = TRUE;
    struct var_scope_t *const scope = var_get_global_scope();

    ARRAY_ITER(it, tree->template_definitions, def, struct vardef_template_t) {
        struct location_t *loc;
        struct vartype_t *type;
        struct var_properties_t *props = var_properties_create();

        props->optional = def->optional;
        var_properties_set_extra(
            props,
            var_extra_create(create_var_flags(FALSE, NULL), clone_var_flags, free)
        );

        if (def->cond) {
            var_properties_set_condition(props, expr_clone(def->cond->expr));
        }

        if (def->defvalue) {
            var_properties_set_default_value(props, expr_clone(def->defvalue->expr));
        }

        if (def->comment) {
            var_properties_set_comment(props, def->comment);
        }

        if (def->type->named) {
            type = vartype_get(def->type->u.id->name);
            if (!type) {
                char *locstr = location_toString(def->location);
                log_error(
                    "Variable type '%s' referenced by the variable template '%s' at %s does not exist.\n",
                    def->type->u.id->name,
                    def->id->name,
                    locstr
                );
                free(locstr);
                var_properties_destroy(props);
                result = FALSE;
                continue;
            }
            else if (type == vartype_get(VARTYPE_PREDEFINED_BOOLEAN)) {
                var_properties_set_disabled_value(props, VALUE_BOOLEAN_FALSE);
            }
            else if (type == vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER)) {
                var_properties_set_disabled_value(props, "0");
            }
        }
        else {
            type = vartype_add_temporary(def->type->u.regex, def->location, TRUE);
            if (!type) {
                char *locstr = location_toString(def->location);
                log_error(
                    "Variable type '%s' referenced by the variable '%s' at %s is invalid.\n",
                    def->type->u.regex,
                    def->id->name,
                    locstr
                );
                free(locstr);
                var_properties_destroy(props);
                result = FALSE;
                continue;
            }
        }

        loc = location_clone(def->location);

        /* first character of def->id->name is a '%' that we want to skip */
        if (!var_template_add(scope, def->id->name + 1, type, VAR_PRIORITY_CONFIGURATION, loc, props)) {
            char *locstr = location_toString(def->location);
            log_error(
                "Adding definition of variable template '%s' at %s failed.\n",
                def->id->name,
                locstr
            );
            free(locstr);
            location_destroy(loc);
            var_properties_destroy(props);
            result = FALSE;
        }
    }

    var_scope_foreach_template(var_get_global_scope(), TRUE,
                               &resolve_template_expressions, &result);

    return result && tree->ok;
}

struct var_t *
check_add_weak_declaration(char const *package, char const *name, int type,
                           BOOL remove, char const *file, int line,
                           int log_level)
{
    char *upper_name;
    struct package_t *pkg;
    struct package_file_t *pkgfile;
    struct location_t *location;
    struct var_properties_t *props;
    struct var_t *var;
    struct vartype_t *vartype;

    switch (type) {
    case TYPE_NUMERIC :
        vartype = vartype_get(VARTYPE_PREDEFINED_UNSIGNED_INTEGER);
        break;
    case TYPE_UNKNOWN:
        vartype = vartype_get(VARTYPE_PREDEFINED_ANYTHING);
        break;
    default:
        fatal_exit("(%s:%d) unknown type %x\n",
                   __FILE__, __LINE__, type);
    }

    upper_name = strsave(convert_to_upper(name));
    var = var_try_get(upper_name);
    if (var) {
        struct var_instance_t *inst = var_instance_create(var, 0, NULL);
        var_instance_clear(inst, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_TRANSIENT);
        var_instance_destroy(inst);

        if (remove) {
            if (var_get_priority(var) == VAR_PRIORITY_SCRIPT_TRANSIENT) {
                var_remove(var);
            }
            else {
                char *locstr = location_toString(var_get_location(var));
                log_error("Error removing non-transient variable '%s' defined at %s in %s, line %d\n",
                          upper_name, locstr, file, line);
                free(locstr);
                free(upper_name);
                return NULL;
            }
        }
        else {
            free(upper_name);
            return var;
        }
    }

    pkg = package_get_or_add(package);
    pkgfile = package_get_file(pkg, PACKAGE_FILE_TYPE_EXTCHECK);
    location = location_create(pkgfile, line, 0);
    props = var_properties_create();
    var_properties_set_extra(
        props,
        var_extra_create(create_var_flags(TRUE, NULL), clone_var_flags, free)
    );
    var = var_add(var_get_global_scope(), upper_name, vartype, VAR_PRIORITY_SCRIPT_TRANSIENT, location, props);
    free(upper_name);

    if (!var) {
        var_properties_destroy(props);
        location_destroy(location);
        return NULL;
    }
    else {
        return var;
    }

    UNUSED(file);
    UNUSED(log_level);
}

#if 0
/*----------------------------------------------------------------------------
 * convert_week_day_to_day ()               - convert week day to index 0 - 6
 *----------------------------------------------------------------------------
 */
static int
convert_week_day_to_day(char const *week_day)
{
    int i;

    for (i = 0; i < DAYS_PER_WEEK; ++i) {
        if (strcmp(week_day, en_wday_strings[i]) == 0 ||
                strcmp(week_day, de_wday_strings[i]) == 0) {
            return (i);
        }
    }

    return -1;
}

/*----------------------------------------------------------------------------
 * init_local_time_table ()                 - initialize local time table
 *----------------------------------------------------------------------------
 */
static void
init_local_time_table(void)
{
    int i;
    for (i = 0; i < DAYS_PER_WEEK*HOURS_PER_DAY; ++i) {
        local_time_table[i] = -1;
    }
}

/*----------------------------------------------------------------------------
 * check_local_time_table () - check whether time table spans the whole week
 *----------------------------------------------------------------------------
 */
static int
check_local_time_table(int times_idx)
{
    int day;
    int hour;
    int start_day = 0;
    int start_hour = 0;
    BOOL found = FALSE;
    int ret = OK;

    for (day = 0; day < DAYS_PER_WEEK; ++day) {
        for (hour = 0; hour < HOURS_PER_DAY; ++hour) {
            if (!found) {
                if (local_time_table[day * HOURS_PER_DAY + hour] == -1) {
                    found = TRUE;
                    ret = ERR;
                    start_day = day;
                    start_hour = hour;
                }
            }
            else {
                if (local_time_table[day * HOURS_PER_DAY + hour] != -1) {
                    found = FALSE;
                    log_error("Error: undefined time span in variable %s starting at %s:%d ending at %s:%d\n",
                              time_variables[times_idx],
                              de_wday_strings[start_day], start_hour,
                              de_wday_strings[day], hour);
                }
            }
        }
    }

    if (found) {
        log_error("Error: undefined time span in variable %s starting at %s:%d ending at %s:%d\n",
                  time_variables[times_idx],
                  de_wday_strings[start_day], start_hour,
                  de_wday_strings[DAYS_PER_WEEK - 1], HOURS_PER_DAY);
    }

    return ret;
}

/*----------------------------------------------------------------------------
 * fill_time_table ()                       - fill time tables
 *----------------------------------------------------------------------------
 */
static int
fill_global_time_table(int times_idx, int start_time, int end_time)
{
    int i;

    for (i = start_time; i < end_time; i++) {
        if (time_table[i] < 0) {
            time_table[i] = times_idx;
        }
        else {
            if (time_table[i] == times_idx) {
                log_error("Error: overlapping time ranges in variable %s\n",
                          time_variables[times_idx]);
            }
            else {
                log_error("Error: overlapping time ranges in variables %s and %s\n",
                          time_variables[time_table[i]], time_variables[times_idx]);
            }
            return ERR;
        }
    }

    return OK;
}

static int
fill_local_time_table(int times_idx, int start_time, int end_time)
{
    int i;

    for (i = start_time; i < end_time; i++) {
        if (local_time_table[i] < 0) {
            local_time_table[i] = 0;
        }
        else {
            log_error("Error: overlapping time ranges in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }
    }
    return OK;
}

static int
fill_time_table(int times_idx, BOOL lcr, int start_time, int end_time)
{
    int res = OK;

    if (lcr) {
        res = fill_global_time_table(times_idx, start_time, end_time);
    }
    if (res == OK) {
        res = fill_local_time_table(times_idx, start_time, end_time);
    }
    return res;
}

/*----------------------------------------------------------------------------
 *  check_a_time_value (int times_idx,
 *----------------------------------------------------------------------------
 */
static int
check_a_time_value(int times_idx, BOOL lcr,
                   char const *start_week_day_str, char const *end_week_day_str,
                   char const *start_hour_str, char const *end_hour_str)
{
    int start_day;
    int end_day;
    int start_hour;
    int end_hour;
    int day;

    start_day = convert_week_day_to_day(start_week_day_str);
    end_day   = convert_week_day_to_day(end_week_day_str);

    if (start_day < 0) {
        log_error("Error: format error in variable %s: no weekday: %s\n",
                  time_variables[times_idx], start_week_day_str);
        return ERR;
    }

    if (end_day < 0) {
        log_error("Error: format error in variable %s: no weekday: %s\n",
                  time_variables[times_idx], end_week_day_str);
        return ERR;
    }

    start_hour = atoi (start_hour_str);
    end_hour   = atoi (end_hour_str);

    if (start_hour < 0 || start_hour >= 24) {
        log_error("Error: format error in variable %s: no valid hour: %s\n",
                  time_variables[times_idx], start_hour_str);
        return ERR;
    }

    if (end_hour <= 0 || end_hour > 24) {
        log_error("Error: format error in variable %s: no valid hour: %s\n",
                  time_variables[times_idx], end_hour_str);
        return ERR;
    }

    day = start_day;

    while (TRUE) {
        int start_time;
        int end_time;

        if (start_hour > end_hour) {
            start_time = day * HOURS_PER_DAY + start_hour;
            end_time   = day * HOURS_PER_DAY + HOURS_PER_DAY;

            if (fill_time_table(times_idx, lcr, start_time, end_time) != OK) {
                return ERR;
            }

            start_time = day * HOURS_PER_DAY + 0;
            end_time   = day * HOURS_PER_DAY + end_hour;

            if (fill_time_table(times_idx, lcr, start_time, end_time) != OK) {
                return ERR;
            }
        }
        else {
            start_time = day * HOURS_PER_DAY + start_hour;
            end_time   = day * HOURS_PER_DAY + end_hour;

            if (fill_time_table(times_idx, lcr, start_time, end_time) != OK) {
                return ERR;
            }
        }

        if (day == end_day) {
            break;
        }

        day = (day + 1) % DAYS_PER_WEEK;
    }

    return OK;
}

/*----------------------------------------------------------------------------
 *  check_time_values (int times_idx, int def_route)    - check time values of variable
 *
 *  Example:
 *
 *  Mo-Fr:08-18:0.032:Y Mo-Fr:18-08:0.025:Y Sa-Su:00-24:0.025:Y
 *----------------------------------------------------------------------------
 */
static int
check_time_values(int times_idx, BOOL def_route)
{
    char *week_range_str;
    char *hour_range_str;
    char *yes_no_str;
    char *start_week_day_str;
    char *end_week_day_str;
    char *start_hour_str;
    char *end_hour_str;
    char *p;
    char *pp;

    char *orig_val;
    BOOL complained = FALSE;

    p = time_values[times_idx];
    orig_val = strsave(p);
    init_local_time_table();

    while (p) {
        char *ppp;

        pp = strchr(p, ' ');
        if (pp != NULL) {
            *pp++ = '\0';

            while (*pp == ' ') {
                ++pp;
            }

            if (!*pp) {
                pp = NULL;
            }
        }

        ppp = strchr(p, ':');
        if (ppp != NULL) {
            week_range_str = p;
            *ppp++ = '\0';
        }
        else {
            log_error("Error: format error in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }

        p = ppp;
        ppp = strchr(p, ':');
        if (ppp != NULL) {
            hour_range_str = p;
            *ppp++ = '\0';
        }
        else {
            log_error("Error: format error in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }

        p = ppp;
        ppp = strchr(p, ':');
        if (ppp != NULL) {
            *ppp++ = '\0';
        }
        else {
            log_error("Error: format error in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }

        yes_no_str = ppp;

        if (strlen(week_range_str) != 5 ||
                *(week_range_str + 2) != '-' ||
                strlen(hour_range_str) != 5 ||
                *(hour_range_str + 2) != '-') {
            log_error("Error: format error in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }

        if (strcmp(yes_no_str, "Y") == 0 || strcmp(yes_no_str, "y") == 0 ||
               strcmp(yes_no_str, "N") == 0 || strcmp(yes_no_str, "n") == 0 ||
               strcmp(yes_no_str, "D") == 0 || strcmp(yes_no_str, "d") == 0) {
            BOOL lcr = strcmp(yes_no_str, "Y") == 0 || strcmp(yes_no_str, "y") == 0;
            *(week_range_str + 2) = '\0';
            start_week_day_str    = week_range_str;
            end_week_day_str      = week_range_str + 3;

            *(hour_range_str + 2) = '\0';
            start_hour_str        = hour_range_str;
            end_hour_str          = hour_range_str + 3;

            if (lcr && !def_route) {
                if (!complained) {
                    log_error("ignoring %s in %s='%s' "
                              "since the circuit has no "
                              "default route and therefore can't be used "
                              "for least cost routing\n",
                              yes_no_str,
                              time_variables[times_idx], orig_val);
                    complained = TRUE;
                }
                lcr = FALSE;
            }

            if (check_a_time_value(times_idx, lcr,
                                   start_week_day_str, end_week_day_str,
                                   start_hour_str, end_hour_str) != OK) {
                return ERR;
            }
        }
        else {
            log_error("Error: format error in variable %s\n",
                      time_variables[times_idx]);
            return ERR;
        }

        p = pp;
    }

    free(orig_val);
    return check_local_time_table(times_idx);
}


/*----------------------------------------------------------------------------
 *  check_time_variables (void)
 *----------------------------------------------------------------------------
 */
static int
check_time_variables(void)
{
    char varname[64];
    char const *p;
    int n_time_variables    = 0;
    int n_pppoe_circuits    = 0;
    BOOL is_pptp             = FALSE;
    int n_isdn_circuits     = 0;
    int times_idx;
    int i;
    int j;
    int ret = OK;

    p = get_variable ("OPT_ISDN");
    if (p && strcmp(p, "yes") == 0) {
        n_isdn_circuits = atoi(get_variable("ISDN_CIRC_N"));
        n_time_variables += n_isdn_circuits;
    }

    p = get_variable("OPT_PPPOE");
    if (p && strcmp(p, "yes") == 0) {
        n_pppoe_circuits = atoi(get_variable("PPPOE_CIRC_N"));
        n_time_variables += n_pppoe_circuits;
    }

    p = get_variable("OPT_PPTP");
    if (p && strcmp (p, "yes") == 0) {
        is_pptp     = TRUE;
        ++n_time_variables;
    }

    if (n_time_variables == 0) {
        return OK;
    }

    time_values    = (char **) malloc(n_time_variables * sizeof(char *));
    time_variables = (char **) malloc(n_time_variables * sizeof(char *));

    for (i = 0; i < n_isdn_circuits; ++i) {
        sprintf(varname, "ISDN_CIRC_%d_TIMES", i + 1);
        time_variables[i] = strsave(varname);
        p = get_variable(varname);
        time_values[i] = strsave(p ? p : "");
    }

    for (; i < n_isdn_circuits + n_pppoe_circuits; ++i) {
        sprintf(varname, "PPPOE_CIRC_%d_TIMES", i - n_isdn_circuits + 1);
        time_variables[i] = strsave(varname);
        p = get_variable(varname);
        time_values[i] = strsave(p ? p : "");
    }

    if (is_pptp) {
        time_variables[i] = strsave("PPTP_TIMES");
        p = get_variable("PPTP_TIMES");
        time_values[i] = strsave(p ? p : "");
    }

    for (i = 0; i < DAYS_PER_WEEK; ++i) {
        for (j = 0; j < HOURS_PER_DAY; ++j) {
            time_table[i * HOURS_PER_DAY + j] = -1;
        }
    }

    for (times_idx = 0; times_idx < n_time_variables; ++times_idx) {
        BOOL def_route = TRUE;
        if (times_idx < n_isdn_circuits) {
            sprintf(varname, "ISDN_CIRC_%d_ROUTE", times_idx + 1);
            p = get_variable(varname);

            if (p && strcmp(p, "default") != 0 && strcmp(p, "0.0.0.0") != 0) {
                def_route = FALSE;              /* no lcr circuit, skip */
            }
        }
        else if (times_idx >= n_isdn_circuits && times_idx < n_isdn_circuits + n_pppoe_circuits) {
            sprintf(varname, "PPPOE_CIRC_%d_DEFAULT", times_idx - n_isdn_circuits + 1);
            p = get_variable(varname);

            if (p && strcmp(p, "yes") != 0) {
                def_route = FALSE;              /* no lcr circuit, skip */
            }
        }

        if (check_time_values (times_idx, def_route) != OK) {
            ret = ERR;
        }
    }

    return ret;
}
#endif

int
check_all_variables(void)
{
#if 0
    if (check_time_variables() != OK) {
        return ERR;
    }
#endif

    return OK;
}
