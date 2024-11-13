/*----------------------------------------------------------------------------
 *  parse.c - functions for parsing
 *
 *  Copyright (c) 2002-2016 - fli4l-Team <team@fli4l.de>
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
 *----------------------------------------------------------------------------
 */

#include <libglob/src/config.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <regex.h>
#include "libcrypt/libcrypt.h"
#include "var.h"
#include "regex.h"
#include "log.h"
#include "str.h"
#include "check.h"
#include "parse.h"
#include "file.h"
#include "tree_struct.h"
#include "package.h"
#include "vartype.h"
#include "var.h"
#include "container/hashmap.h"
#include "expression/expr_match.h"

#define ENTRY_OPT_ARRAY_SIZE    4

/**
 * Describes a service, its version and the providing package.
 */
struct service_t {
    /**
     * The service name.
     */
    char *name;
    /**
     * The service version.
     */
    char *version;
    /**
     * The package providing the service.
     */
    char *provider;
};

/**
 * Describes the dependencies of a package.
 */
struct depender_t {
    /**
     * The package name.
     */
    char *name;
    /**
     * Array of dependencies of type dependency_t.
     */
    array_t *deps;
};

/**
 * Describes a single dependency.
 */
struct dependency_t {
    /**
     * The service a package depends upon.
     */
    char *service_name;
    /**
     * An array of required service versions. If empty, any service version
     * satisfies the dependency.
     */
    array_t *service_versions;
};

static BOOL exec_failed = FALSE;
static struct hashmap_t *service_map = NULL;
static struct hashmap_t *depender_map = NULL;
static char *parse_current_package;
static char *parse_current_file;
static char var_buf[VAR_SIZE + 1];

extern FILE *yyin;
void yyrestart(FILE *fp);

/**
 * Adds a dependency provider --> (service_name, service_version). If some
 * other package already provides a service with the same name, an error is
 * returned.
 *
 * @param provider
 *  The package providing a service.
 * @param service_name
 *  The name of the provided service.
 * @param service_version
 *  The version of the provided service.
 * @return
 *  TRUE if successful, FALSE if another package already provides a service
 *  with the same name.
 */
static BOOL add_provider(char const *provider, char const *service_name, char const *service_version) {
    struct service_t *service = hashmap_get(service_map, &service_name);
    if (service) {
        if (strcmp(service->provider, provider) != 0) {
            log_error("Service '%s' provided by both '%s' and '%s' packages.\n", service_name, service->provider, provider);
            return FALSE;
        }
        else {
            return TRUE;
        }
    }
    else {
        struct service_t *service = (struct service_t *) safe_malloc(sizeof(struct service_t));
        service->name = strsave(service_name);
        service->version = strsave(service_version);
        service->provider = strsave(provider);
        hashmap_put(service_map, service);

        log_info(LOG_INFO, "Service ('%s','%s') provided by package '%s'.\n",
                 service_name, service_version, provider);
        return TRUE;
    }
}

/**
 * Frees a service descriptor.
 *
 * @param entry
 *  The service descriptor.
 */
static void free_service(void *entry) {
    struct service_t *service = (struct service_t *) entry;
    free(service->name);
    free(service->version);
    free(service->provider);
    free(service);
}

/**
 * Adds a dependency depender --> (service_name, service_version). After all
 * extended check scripts have been executed, it is checked whether the
 * dependency can be satisfied by some package.
 *
 * @param package
 *  The package requesting a service.
 * @param service_name
 *  The name of the requested service.
 * @param service_version
 *  The version of the requested service. If it is NULL, any service version
 *  will satisfy the dependency.
 */
static void add_dependency(char const *package, char const *service_name, char const *service_version) {
    DECLARE_ARRAY_ITER(it, dep, struct dependency_t);
    struct depender_t *depender = hashmap_get(depender_map, &package);
    if (!depender) {
        depender = (struct depender_t *) safe_malloc(sizeof(struct depender_t));
        depender->name = strsave(package);
        depender->deps = init_array(1);
        hashmap_put(depender_map, depender);
    }

    ARRAY_ITER_LOOP(it, depender->deps, dep) {
        if (strcmp(dep->service_name, service_name) == 0) {
            break;
        }
    }

    if (!dep) {
        dep = (struct dependency_t *) safe_malloc(sizeof(struct dependency_t));
        dep->service_name = strsave(service_name);
        dep->service_versions = init_array(1);
        append_new_elem(depender->deps, dep);
    }

    if (service_version) {
        append_new_elem(dep->service_versions, strsave(service_version));
    }
}

/**
 * Frees a depender descriptor.
 *
 * @param entry
 *  The depender descriptor.
 */
static void free_depender(void *entry) {
    DECLARE_ARRAY_ITER(it, dep, struct dependency_t);
    struct depender_t *depender = (struct depender_t *) entry;
    free(depender->name);
    ARRAY_ITER_LOOP(it, depender->deps, dep) {
        DECLARE_ARRAY_ITER(it2, ver, char);
        free(dep->service_name);
        ARRAY_ITER_LOOP(it2, dep->service_versions, ver) {
            free(ver);
        }
        free_array(dep->service_versions);
        free(dep);
    }
    free_array(depender->deps);
    free(depender);
}

/**
 * Checks whether the dependencies of a package are satisfied.
 *
 * @param entry
 *  A depender descriptor.
 * @param data
 *  Points to a boolean flag. If some dependency cannot be satisfied, this flag
 *  is set to FALSE.
 * @return
 *  Always TRUE to continue the checking process for subsequent packages even
 *  if some checks fail.
 */
static BOOL check_depender(void *entry, void *data)
{
    DECLARE_ARRAY_ITER(it, dep, struct dependency_t);
    struct depender_t *depender = (struct depender_t *) entry;
    BOOL *ok = (BOOL *) data;

    ARRAY_ITER_LOOP(it, depender->deps, dep) {
        DECLARE_ARRAY_ITER(it2, ver, char);
        BOOL found = FALSE;
        size_t service_version_len;
        int num_versions = 0;
        struct service_t *service = hashmap_get(service_map, &dep->service_name);
        if (!service) {
            log_error(
                "Dependency '%s' --> '%s' cannot be satisfied by any package.\n",
                depender->name,
                dep->service_name
            );
            *ok = FALSE;
            continue;
        }

        service_version_len = strlen(service->version);
        ARRAY_ITER_LOOP(it2, dep->service_versions, ver) {
            size_t const dep_version_len = strlen(ver);
            ++num_versions;
            if (dep_version_len > service_version_len) {
                continue;
            }
            if (strncmp(service->version, ver, dep_version_len) == 0) {
                if (service->version[dep_version_len] == '\0' ||
                        service->version[dep_version_len] == '.') {
                    log_info(
                        LOG_INFO,
                        "Dependency '%s' --> ('%s', '%s') resolved by ('%s', '%s').\n",
                        depender->name,
                        dep->service_name,
                        ver,
                        service->provider,
                        service->version
                    );
                    found = TRUE;
                    break;
                }
            }
        }

        if (!found) {
            if (num_versions > 0) {
                log_error(
                    "Dependency '%s' --> '%s' cannot be satisfied by package '%s' due to incompatible versions.\n",
                    depender->name,
                    dep->service_name,
                    service->provider
                );
                *ok = FALSE;
            }
            else {
                log_info(
                    LOG_INFO,
                    "Dependency '%s' --> '%s' resolved by ('%s', '%s').\n",
                    depender->name,
                    dep->service_name,
                    service->provider,
                    service->version
                );
            }
        }
    }

    return TRUE;
}

static void
parse_set_current(char const *pkg, char const *file)
{
    parse_current_package = strsave(pkg);
    parse_current_file = strsave(file);
}

static BOOL
parse_get_error(void)
{
    return exec_failed;
}

static void
parse_set_error(void)
{
    exec_failed = TRUE;
}

char const *
parse_get_variable(char const *name, char const *package, int line)
{
    char const *content = get_variable(convert_to_upper(name));
    if (!content) {
        log_error("unknown variable '%s' in %s, line %d\n", name, package, line);
        parse_set_error();
        return NULL;
    }
    else {
        return content;
    }
}

static char const *
parse_get_variable_package(char const *name, char const *package, int line)
{
    char const *content = get_variable_package(convert_to_upper(name));
    if (!content) {
        log_error("unknown variable '%s' in %s, line %d\n", name, package, line);
        parse_set_error();
        return NULL;
    }
    else {
        return content;
    }
}

static char const *
parse_get_variable_comment(char const *name, char const *package, int line)
{
    char const *content = get_variable_comment(convert_to_upper(name));
    if (!content) {
        log_error("unknown variable '%s' in %s, line %d\n", name, package, line);
        parse_set_error();
        return NULL;
    }
    else {
        return content;
    }
}

array_t *
parse_entry_options(char const *opts)
{
    array_t *result = init_array(ENTRY_OPT_ARRAY_SIZE);
    char const *p = opts;
    char const *q;

    while ((q = strchr(p, '=')) != NULL) {
        int const name_len = q - p;
        struct opt_t *opt = (struct opt_t *) safe_malloc(sizeof(struct opt_t));
        append_new_elem(result, opt);

        /* read name */
        opt->name = (char *) safe_malloc(name_len + 1);
        strncpy(opt->name, p, name_len);
        opt->name[name_len] = '\0';

        /* read value */
        p = q + 1;
        q = strchr(p, ' ');
        if (q != NULL) {
            int const value_len = q - p;
            opt->value = (char *) safe_malloc(value_len + 1);
            strncpy(opt->value, p, value_len);
            opt->value[value_len] = '\0';
        } else {
            opt->value = strsave(p);
        }
        p += strlen(opt->value);

        /* skip trailing spaces */
        while (*p && isspace((int) *p))
            ++p;
    }

    return result;
}

#define PMAX_MATCH 20
BOOL
str_strmatch(char const *value, char const *expr, char const *package, char const *file, int line)
{
    int res;
    regmatch_t match[PMAX_MATCH];
    char *match_set_var = "MATCH_%";
    struct var_t *var;
    struct var_instance_t *inst;

    var = check_add_weak_declaration(package, match_set_var, TYPE_UNKNOWN,
                                     FALSE, file, line, LOG_TREE_EXEC);
    if (!var) {
        log_error("unexpected error while creating variable %s\n", match_set_var);
        parse_set_error();
        return FALSE;
    }

    inst = var_instance_create(var, 1, NULL);
    if (!inst) {
        log_error("unexpected error while creating instance of variable %s\n", match_set_var);
        parse_set_error();
        return FALSE;
    }

    res = regexp_user(value, expr, PMAX_MATCH, match, 0, package, line);
    log_info(LOG_TREE_EXEC, "checking regular expression '%s' against value '%s', "
             "res = %d\n", expr, value, res);

    if (res == REGEX_MATCHED) {
        int i;
        for (i = 1; i < PMAX_MATCH && match[i].rm_so != -1; ++i) {
            int const len = match[i].rm_eo - match[i].rm_so;
            char *substr = (char *) safe_malloc(len + 1);
            strncpy(substr, value + match[i].rm_so, len);
            substr[len] = '\0';

            var_instance_set_and_check_value(inst, substr, NULL,
                                             VAR_ACCESS_WEAK,
                                             VAR_PRIORITY_SCRIPT_TRANSIENT);
            free(substr);
            ++inst->indices[0];
        }

        var_instance_destroy(inst);
        return TRUE;
    }
    else if (res == REGEX_NOT_MATCHED) {
        var_instance_destroy(inst);
        return FALSE;
    }
    else {
        log_error("wrong regular expression '%s' in %s, line %d\n",
                  expr, package, line);
        parse_set_error();
        var_instance_destroy(inst);
        return FALSE;
    }
}

int
val_numcmp(char const *val, char const *num, char const *file, int line)
{
    unsigned long x, y;

    log_info(LOG_TREE_EXEC, "numcmp ('%s' rel %s') ?\n", val, num);

    x = convert_to_long(val, file, line);
    y = convert_to_long(num, file, line);

    if (x == y) {
        return CMP_EQUAL;
    }
    else if (x < y) {
        return CMP_LESS;
    }
    else {
        return CMP_GREATER;
    }
}

static int
get_ver_num(char const *ver, char const *file, int line)
{
    int ver_major, ver_minor;
    unsigned long sub = 0;

    if (!ver) {
        fatal_exit("%s %d: Null pointer passed to get_ver_num\n",
                   __FILE__, __LINE__);
    }
    if (strlen(ver) < 3) {
        fatal_exit("%s %d: Short version passed to get_ver_num\n",
                   __FILE__, __LINE__);
    }

    /* kristov TODO: this algorithm only works with major and minor < 10! */
    ver_major = ver[0] - '0';
    ver_minor = ver[2] - '0';

    if (ver[3] != '\0') {
        sub = convert_to_long(&ver[4], file, line);
    }
    return sub + ver_minor * 1000 + ver_major * 10000 ;
}

int
val_vercmp(char const *val, char const *version, char const *file, int line)
{
    int x, y;

    log_info(LOG_TREE_EXEC, "vercmp ('%s'=='%s') ?\n", val, version);

    x = get_ver_num(val, file, line);
    y = get_ver_num(version, file, line);

    if (x == y) {
        return CMP_EQUAL;
    }
    else if (x < y) {
        return CMP_LESS;
    }
    else {
        return CMP_GREATER;
    }
}

char *
parse_rewrite_string(char const *msg, char const *package, int line)
{
    size_t msg_size = 8192;
    char *msg_buf = safe_malloc(msg_size);
    char *p=msg_buf;

    while (*msg) {
        switch (*msg) {
        case '$':
        case '%':
        case '@':
            if (isalpha((int) *(msg + 1)) || *(msg + 1) == '{') {
                /* get variable name */
                char *var = var_buf;
                char prefix = *msg++;
                char const *value = NULL;

                if (*msg == '{') {
                    ++msg;
                    while ((isalnum((int) *msg) || *msg == '_') && *msg != '}') {
                        *var++ = *msg++;
                    }
                    ++msg;
                }
                else {
                    while (isalnum((int) *msg) || *msg == '_') {
                        *var++ = *msg++;
                    }
                }
                *var = '\0';
                if (strlen(var_buf) >= VAR_SIZE) {
                    fatal_exit("%s %d: buffer overflow,"
                               " variable name too long\n",
                               __FILE__, __LINE__);
                }

                switch (prefix) {
                case '$':
                    value = parse_get_variable(var_buf, package, line);
                    break;
                case '%':
                    value = parse_get_variable_package(var_buf, package, line);
                    break;
                case '@':
                    value = parse_get_variable_comment(var_buf, package, line);
                    break;
                }
                /* append to msg text */
                *p = '\0';
                if (value) {
                    strcat(msg_buf, value);
                    p += strlen(value);
                }

                break;
            }
            else {
                switch (*(msg + 1)) {
                case '$':
                case '%':
                case '@':
                    ++msg;
                }
                /* fall through */
            }
        default:
            *p++ = *msg++;
            break;
        }
    }
    *p = '\0';
    if (strlen(msg_buf) >= msg_size) {
        fatal_exit("%s %d: buffer overflow, msg too long\n",
                   __FILE__, __LINE__);
    }
    return msg_buf;
}

void
parse_warning(char const *warning, char const *package, int line)
{
    char *p = parse_rewrite_string(warning, package, line);
    log_error("Warning: %s\n", p);
    free(p);

}
void
parse_error(char const *warning, char const *package, int line)
{
    char *p = parse_rewrite_string(warning, package, line);
    log_error("Error: %s\n", p);
    free(p);
    parse_set_error();
}

void
parse_fatal_error(char const *warning, char const *package, int line)
{
    char *p = parse_rewrite_string(warning, package, line);
    log_error("Fatal Error: %s\n", p);
    free(p);
    fatal_exit ("fatal error, aborting...\n");
}

void
parse_add_to_opt(char const *file, char const *options, char const *package, int line)
{
    if (add_to_zip_list("OPT_BASE", "yes", file,
                        options ? parse_entry_options(parse_rewrite_string(options, package, line)) : NULL,
                        package, line) != OK) {
        parse_set_error();
    }
}

void
parse_crypt(char const *id, char const *package, char const *pkgfile, int line)
{
    struct var_instance_t *inst;
    struct var_value_t *value;

    inst = var_instance_create_from_string(var_get_global_scope(), id);
    if (!inst) {
        log_error("unknown variable '%s' in %s, line %d\n", id, pkgfile, line);
        parse_set_error();
        return;
    }

    value = var_instance_get_value(inst);
    if (value) {
        char *crypted_passwd;
        char salt[8 + 1];
        unsigned i;
        static char const rndChar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";

        for (i = 0; i < sizeof(salt) - 1; ++i) {
            salt[i] = rndChar[rand() % 64];
        }
        salt[i] = '\0';
        crypted_passwd = __sha256_crypt(value->value, salt);
        if (!var_instance_set_and_check_value(inst, crypted_passwd, NULL, VAR_ACCESS_WEAK, VAR_PRIORITY_SCRIPT_PERSISTENT)) {
            log_error("(%s:%d) Error setting variable '%s' to '%s'", pkgfile, line,
                      id, crypted_passwd);
        }
    }

    var_instance_destroy(inst);
    UNUSED(package);
}

void
parse_stat(char const* file, char const *id, char const *package, char const *pkgfile, int line)
{
    char *p;
    struct stat stat_buf;

    p = parse_rewrite_string(file, package, line);
    log_info (LOG_TREE_EXEC, "executing stat on %s, results go to %s_*\n",
              p, id);

    strcpy(var_buf, id);
    if (stat(p, &stat_buf) != 0) {
        strcat(var_buf, "_RES");
        var_add_weak_declaration(package, var_buf,
                                 strsave(strerror(errno)), NULL, NULL,
                                 TYPE_UNKNOWN, pkgfile, line, LOG_TREE_EXEC,
                                 PACKAGE_FILE_TYPE_EXTCHECK);
    }
    else {
        char buf[128];
        char *end = var_buf + strlen(var_buf);
        strcpy(end, "_RES");
        var_add_weak_declaration(package, var_buf,
                                 strsave("OK"), NULL, NULL, TYPE_UNKNOWN,
                                 pkgfile, line, LOG_TREE_EXEC,
                                 PACKAGE_FILE_TYPE_EXTCHECK);
        sprintf(buf, "%ld", (unsigned long) stat_buf.st_size);
        strcpy(end, "_SIZE");
        var_add_weak_declaration(package, var_buf,
                                 strsave(buf), NULL, NULL, TYPE_NUMERIC,
                                 pkgfile, line, LOG_TREE_EXEC,
                                 PACKAGE_FILE_TYPE_EXTCHECK);
    }
    free(p);
}

void
parse_fgrep(char const *file, char const *_expr, char const *package, char const *pkgfile, int line)
{
    char *p;
    char *expr;
    int res;
    char *match_set_var = "FGREP_MATCH_%";
    FILE *fh;
    glob_t glob_res;
    struct var_t *var;
    struct var_instance_t *inst;

    p = parse_rewrite_string(file, package, line);
    expr = parse_rewrite_string(_expr, package, line);

    log_info(LOG_TREE_EXEC, "checking regular expression '%s' against file '%s'\n", expr, p);

    var = check_add_weak_declaration(package, match_set_var, TYPE_UNKNOWN,
                                     FALSE, pkgfile, line, LOG_TREE_EXEC);
    if (!var) {
        fatal_exit("(%s:%d) unexpected error while creating variable %s\n",
                   __FILE__, __LINE__, match_set_var);
    }

    inst = var_instance_create(var, 1, NULL);
    if (!inst) {
        fatal_exit("(%s:%d) unexpected error while creating instance of variable %s\n",
                   __FILE__, __LINE__, match_set_var);
    }

    if (glob(p, 0, NULL, &glob_res) == 0) {
        size_t i;
        for (i = 0; i < glob_res.gl_pathc; ++i) {
            char *found = glob_res.gl_pathv[i];
            fh = fopen(found, "r");
            if (fh != NULL) {
                char buff[1024];

                while(fgets(buff, sizeof(buff), fh) != NULL) {
                    regmatch_t match[PMAX_MATCH];
                    int len;

                    len = strlen(buff);
                    if (len > 1) {
                         --len;
                    }
                    if (buff[len] == '\n' || buff[len] == '\r') {
                        buff[len] = '\0';
                    }
                    if (len > 1) {
                        --len;
                    }
                    if (buff[len] == '\n' || buff[len] == '\r') {
                        buff[len] = '\0';
                    }

                    res = regexp_user(buff, expr, PMAX_MATCH, match, 0, pkgfile, line);
                    if (res == REGEX_MATCHED) {
                        int i;

                        for (i=0; i<PMAX_MATCH && match[i].rm_so != -1; i++)
                          {
                            char * end = buff+match[i].rm_eo;
                            char tmp = *end;
                            *end = '\0';

                            var_instance_set_and_check_value(inst, buff+match[i].rm_so,
                                                             NULL,
                                                             VAR_ACCESS_WEAK,
                                                             VAR_PRIORITY_SCRIPT_TRANSIENT);
                            ++inst->indices[0];
                            *end = tmp;
                          }
                    }
                    else if (res != REGEX_NOT_MATCHED) {
                        fatal_exit("wrong regular expression in fgrep '%s' in %s, line %d, terminating\n",
                                   expr, pkgfile, line);
                    }
                }

                fclose(fh);
            }
        }
    }

    var_instance_destroy(inst);
    free(p);
    free(expr);
}

void
parse_provides(char const *id, char const *version, char const *package, char const *pkgfile, int line)
{
    if (!add_provider(convert_to_upper(package), id, version)) {
        parse_set_error();
    }
    UNUSED(pkgfile);
    UNUSED(line);
}

void
parse_depends(char const *id, elem_t *version, char const *package,
              char const *pkgfile, int line)
{
    if (!version) {
        add_dependency(convert_to_upper(package), id, NULL);
    }
    else {
        while (version) {
            char const *const value = version->ARG[0]->VAL;
            add_dependency(convert_to_upper(package), id, value);
            version = version->ARG[1];
        }
    }

    UNUSED(pkgfile);
    UNUSED(line);
}

/**
 * Parses an extended check script.
 *
 * @param package
 *  The package being processed.
 * @param file
 *  The extended check script to be parsed.
 * @return
 *  TRUE if parsing the extended check script was successful, FALSE
 *  otherwise.
 */
BOOL
parse_check_file(struct package_file_t *file, struct package_t *package)
{
    char const *check_file = package_file_get_name(file);
    FILE *f = fopen(check_file, "r");
    if (f) {
        int ret;

        yyrestart(f);
        yyline = 1;
        inc_log_indent_level();
        parse_set_current(package_get_name(package), check_file);
        ret = yyparse();
        dec_log_indent_level();
        fclose (f);

        if (ret == OK) {
            return TRUE;
        }
        else {
            log_error("Error while parsing check file %s.\n", check_file);
            return FALSE;
        }
    }
    else {
        log_error("Error opening extended check file %s.\n", check_file);
        return FALSE;
    }
}

char const *
parse_get_current_package(void)
{
    return parse_current_package;
}

char const *
parse_get_current_file(void)
{
    return parse_current_file;
}

BOOL
execute_all_extended_checks(void)
{
    BOOL result = TRUE;

    /* reorder scripts for compatibility */
    reorder_check_scripts();

    service_map = hashmap_create(
        offsetof(struct service_t, name),
        16,
        hash_ci_string,
        compare_ci_string_keys,
        &free_service
    );

    depender_map = hashmap_create(
        offsetof(struct depender_t, name),
        16,
        hash_ci_string,
        compare_ci_string_keys,
        &free_depender
    );

    inc_log_indent_level();
    walk_tree();
    dec_log_indent_level();

    if (parse_get_error()) {
        result = FALSE;
    }
    else {
        log_info(LOG_INFO, "resolving service dependencies\n");
        inc_log_indent_level();
        hashmap_foreach(depender_map, &check_depender, &result);
        dec_log_indent_level();
    }

    hashmap_destroy(depender_map);
    hashmap_destroy(service_map);
    return result;
}
