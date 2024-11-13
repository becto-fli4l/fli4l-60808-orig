/*----------------------------------------------------------------------------
 *  options.c - functions for handling global mkfli4l options
 *
 *  Copyright (c) 2003-2016 - fli4l-Team <team@fli4l.de>
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#if !defined(PLATFORM_mingw32)
#include <sys/utsname.h>
#endif
#include "log.h"
#include "parse.h"
#include "cfg.h"
#include "options.h"

static int      running_as_mkfli4l (char * name);
static void     show_help (void);
static void     set_debug_option (char * option);
static void     set_default_values (void);

char * opt_file        = "opt/opt.txt";

char * build_dir = "img";
char * scratch_dir = "img";
char * distrib_dir = "img";

char   full_rc_file[1024];
char   rc_file[1024];
char const * rootfs_img_file = "rootfs.img";
char const * opt_img_file = "opt.img";

char * syslinux_template_file_name = "syslinux.tmpl";
char * syslinux_cfg_file_name      = "syslinux.cfg";
char   syslinux_template_file[1024];
char   modules_dep_file[1024];
char   modules_alias_file[1024];
char * modules_dep_name = "modules.dep";
char * modules_alias_name = "modules.alias";
char   syslinux_cfg_file[1024];

char * config_dir;
char * config_dir_with_slashes;
int config_dir_len;

char * check_dir;
char * logfile;
char *  def_cfg_ext     = ".txt";
char *  def_opt_ext     = ".txt";
char *  def_check_ext   = ".txt";
char *  def_regex_ext   = ".exp";
char *  def_extcheck_ext= ".ext";

int strong_consistency;
int no_squeeze = 0;

static int num_opt_packages = 0;
char * opt_packages [20];

int check_opt_files = 0;
int is_mkfli4l = 0;
static int log_level = 0;

int running_as_mkfli4l (char * name)
{
    char * p = strrchr (name, '/');

    if (!p)
    {
        p = name;
    }
    p = strstr (p, "eischk");
    return !p;
}

void
show_help (void)
{
#if !defined(PLATFORM_mingw32)
    struct utsname sys;

    if (uname (&sys) != -1)
    {
        printf ("%s (%s) running under %s Version %s\n",
                is_mkfli4l ? "mkfli4l" : "eischk",
                BUILD_DATE, sys.sysname, sys.release);
    }
#else
    printf ("%s (%s)\n", is_mkfli4l ? "mkfli4l" : "eischk", BUILD_DATE);
#endif
    printf ("usage: %s [options]\n\n"
            "\t-c, --config\t- set config directory (default: %s)\n"
            "\t-x, --check\t- set check directory (default: %s)\n"
            "\t-b, --build\t- set build directory (default: %s)\n"
            "\t-t, --tmp\t- set tmp directory (default: none)\n"
            "\t-l, --log\t- set log file (default: %s)\n"
            "\t-p, --package\t- specify package to check\n"
            "\t-s, --strong\t- check consistency between check file\n"
            "\t\t\t  and configuration\n"
            "\t-w, --weak\t- ignore inconsistencies between check file\n"
            "\t\t\t  and configuration\n"
            "\t-n, --no-squeeze\t- don't squeeze shelle scripts\n",
            is_mkfli4l ? "mkfli4l" : "eischk",
            config_dir, check_dir, build_dir, logfile);
    printf (
            "\t-i, --info\t- show some traces during execution\n"
            "\t-v, --verbose\t- show verbose trace of execution\n"
            "\t-h, --help\t- show this help\n"
            "\t-d, --debug\t- specify debug option\n");
#ifdef DEBUG
    printf(
            "\t    scan\t- show scan trace\n"
            "\t    yacc\t- show yacc trace\n");
#endif
    printf(
            "\t    ext-trace\t- show trace of extended checks\n"
            "\t    ext-tree\t- show generation of parse tree\n"
            "\t    check\t- show check process\n"
            "\t    regex\t- show regex trace\n"
            "\t    exp\t- show regexp reading\n"
            "\t    zip-list\t- show generation of zip list\n"
            "\t    zip-list-skipped\t- show skipped files\n"
            "\t    zip-list-regexp\t- show regular expressions for ziplist\n");
    printf(
            "\t    opt-files\t- check all files in opt/package.txt\n"
            "\t    dep\t- show automatic kernel module dependency processing\n"
            "\t    lib-dep\t- show automatic library dependency processing\n"
            "\t    lib-symlink\t- show creation of library symlinks\n"
            "\t    uidgid\t- show uid and gid entries processed\n"
            "\t    archive\t- show creation of archives\n");
}

void set_debug_option (char * option)
{
    char ** p;
    char * option_names[] = {
        "scan", "yacc",
        "ext-trace", "ext-tree",
        "check",
        "zip-list", "zip-list-skipped", "zip-list-regexp",
        "opt-files", "regex", "exp", "dep", "lib-dep", "lib-symlink",
        "uidgid", "archive", NULL };

    for (p=option_names; *p && strcmp (*p, option); p++)
        ;

    switch (p-option_names)
    {
#ifdef DEBUG
    case 0:
        yy_flex_debug = 1;
        cfg_flex_debug = 1;
        log_level |= LOG_SCAN;
        break;
    case 1:
        yydebug = 1;
        break;
#endif
    case 2:
        log_level |= LOG_TREE_EXEC;
        break;
    case 3:
        log_level |= LOG_TREE_BUILD;
        break;
    case 4:
        log_level |= LOG_VAR;
        break;
    case 5:
        log_level |= LOG_ZIPLIST;
        break;
    case 6:
        log_level |= LOG_ZIPLIST_SKIP;
        break;
    case 7:
        log_level |= LOG_ZIPLIST_REGEXP;
        break;
    case 8:
        check_opt_files = 1;
        break;
    case 9:
        log_level |= LOG_REGEXP;
        break;
    case 10:
        log_level |= LOG_EXP;
        break;
    case 11:
        log_level |= LOG_DEP;
        break;
    case 12:
        log_level |= LOG_LIB_DEP;
        break;
    case 13:
        log_level |= LOG_LIB_SYMLINK;
        break;
    case 14:
        log_level |= LOG_UIDGID;
        break;
    case 15:
        log_level |= LOG_ARCHIVE;
        break;
    default:
        fprintf (stderr, "unknown option %s\n", option);
        show_help ();
        exit (1);
    }
    no_reformat = TRUE;
}

void set_default_values (void)
{
#ifdef DEBUG
    yy_flex_debug = 0;
    cfg_flex_debug = 0;
    yydebug = 0;
#endif

    if (is_mkfli4l)
    {
        config_dir  = "config";
        check_dir   = "check";
        logfile     = "img/mkfli4l.log";
        strong_consistency = 1;
    }
    else
    {
        config_dir  = "config.d";
        check_dir   = "check.d";
        logfile     = "/var/log/eischk/eischk.log";
        def_cfg_ext = def_check_ext = NULL;
        strong_consistency = 1;
    }
}

int
cmp_package(void const *first, void const *second)
{
    return strcmp(*(char const **) first, *(char const **) second);
}

int
get_options (int argc, char **argv)
{
    int c, option_index;
#if defined(PLATFORM_mingw32)
    int i;
#endif
    static struct option long_options[] =
    {
        {"config",              required_argument, 0, 'c'},
        {"check",               required_argument, 0, 'x'},
        {"build",               required_argument, 0, 'b'},
        {"tmp",                 required_argument, 0, 't'},
        {"log",                 required_argument, 0, 'l'},
        {"package",             required_argument, 0, 'p'},
        {"strong",              no_argument, 0, 's'},
        {"weak",                no_argument, 0, 'w'},
        {"verbose",             no_argument, 0, 'v'},
        {"info",                no_argument, 0, 'i'},
        {"help",                no_argument, 0, 'h'},
        {"debug",               required_argument, 0, 'd'},
        {"no-squeeze",          no_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    is_mkfli4l = running_as_mkfli4l (argv[0]);
    opt_packages[0] = NULL;
    set_default_values ();

    while (1)
    {
        c = getopt_long (argc, argv, "c:x:b:t:l:p:d:swvihn",
                       long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'c':
            config_dir = optarg;
            break;
        case 'x':
            check_dir = optarg;
            break;
        case 'l':
            logfile = optarg;
            break;
        case 'b':
            build_dir = optarg;
            break;
        case 't':
            scratch_dir = optarg;
            break;
        case 'p':
            opt_packages[num_opt_packages++] = optarg;
            opt_packages[num_opt_packages] = NULL;
            break;
        case 'd':
            set_debug_option (optarg);
            break;
        case 'i':
            log_level |= LOG_INFO;
            break;
        case 'v':
            log_level |= LOG_INFO|LOG_VERBOSE;
            break;
        case 's':
            strong_consistency = 1;
            break;
        case 'w':
            strong_consistency = 0;
            break;
        case 'n':
            no_squeeze = 1;
            break;
        case 'h':
            show_help ();
            exit (0);
            break;
        case '?':
            show_help ();
            exit (1);
            break;
        }
    }

    if ( (optind != argc) || (!is_mkfli4l && !*opt_packages) )
    {
        show_help ();
        exit (1);
    }

    set_log_level (log_level);

    snprintf (syslinux_template_file, 1023, "%s/%s", distrib_dir,
              syslinux_template_file_name);
    snprintf (modules_dep_file, 1023, "%s/%s", scratch_dir,
              modules_dep_name);
    snprintf (modules_alias_file, 1023, "%s/%s", scratch_dir,
              modules_alias_name);
    snprintf (syslinux_cfg_file, 1023, "%s/%s", build_dir,
              syslinux_cfg_file_name);
    snprintf (full_rc_file, 1023, "%s/full_rc.cfg", build_dir);

#ifdef DEBUG
    log_info (LOG_INFO, "setting log level to %x, yydebug=%d, yy_flex_debug=%d\n",
              log_level, yydebug, yy_flex_debug);
#endif

    config_dir_len = strlen(config_dir);
    while (config_dir[config_dir_len - 1] == '/')
        config_dir[--config_dir_len] = 0;

#if defined(PLATFORM_mingw32)
    /* needed for glob() */
    config_dir_with_slashes = strdup(config_dir);
    for (i = 0; i < config_dir_len; ++i) {
        if (config_dir_with_slashes[i] == '\\') {
            config_dir_with_slashes[i] = '/';
        }
    }
#else
    config_dir_with_slashes = config_dir;
#endif

    snprintf (rc_file, 1023, "%s/etc/rc.cfg", config_dir);
    qsort(opt_packages, num_opt_packages, sizeof(char *), cmp_package);
    return optind;
}

