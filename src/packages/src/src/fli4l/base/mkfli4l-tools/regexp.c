/*----------------------------------------------------------------------------
 *  regexp.c - regular expression tool
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
 *  Creation:    2002-03-01  jw5
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <stdarg.h>
#include <dirent.h>
#include <libmkfli4l/var.h>
#include <libmkfli4l/regex.h>
#include <libmkfli4l/check.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/cfg.h>
#include <libmkfli4l/parse.h>
#include <package.h>
#include <vartype.h>
#include <var.h>

int n_packages;
char *  packages[10];


int
add_to_zip_list(char const *var, char const *content, char const *filename, array_t *opt, char const *opt_file, int line)
{
    fprintf (stderr, "invoked add_to_zip_list");
    abort();
    (void) var;
    (void) content;
    (void) filename;
    (void) opt;
    (void) opt_file;
    (void) line;
}

void read_check_files (char * check_dir, int read_base_exp);
void show_help (int level);

void read_check_files (char * check_dir, int read_base_exp)
{
    DIR *               dirp;
    struct dirent *     dp;
    char                buf[512];
    int                 len;
    struct package_t *package;
    struct package_file_t *file;

    dirp = opendir (check_dir);

    if (! dirp)
    {
        fatal_exit ("Error opening check dir '%s': %s\n",
                    check_dir, strerror (errno));
    }

    if (read_base_exp)
    {
        sprintf (buf, "%s/%s", check_dir, "base.exp");
        log_info (LOG_INFO|LOG_VERBOSE, "reading %s\n", buf);
        package = package_get_or_add("base");
        file = package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, buf);
        (void)regexp_read_file (file, package);
    }

    while ((dp = readdir (dirp)) != (struct dirent *) NULL)
    {
        len = strlen (dp->d_name);

        if (len > 4 && ! strcasecmp (dp->d_name + len - 4, ".exp") &&
            strcasecmp (dp->d_name, "base.exp"))
        {
            sprintf (buf, "%s/%s", check_dir, dp->d_name);
            dp->d_name[len - 4] = '\0';
            log_info (LOG_INFO|LOG_VERBOSE, "reading %s\n", buf);
            package = package_get_or_add(dp->d_name);
            file = package_set_file(package, PACKAGE_FILE_TYPE_VARTYPE, buf);
            (void)regexp_read_file (file, package);
        }
    }

    (void) closedir (dirp);
}

int main (int argc, char ** argv)
{
    char *check_dir         = "./check";

    int read_base_exp = 1;
    int separator = 0;
    int res;

    cfg_flex_debug = 0;
    argv++;
    argc--;
    no_reformat = 1;

    package_init_module();
    vartype_init_module();
    var_init_module();
    var_init();
    check_init();
    regexp_init();

    while (argc)
    {
        if (!strcmp (*argv, "-c") && argc  >= 2)
        {
            read_check_files (argv[1], read_base_exp);
            read_base_exp = 0;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp (*argv, "-h"))
        {
            argc--;
            argv++;
            show_help (1);
        }
        else if (!strcmp (*argv, "-v"))
        {
            argc--;
            argv++;
            set_log_level (LOG_INFO);
        }
        else if (!strcmp (*argv, "-vv"))
        {
            argc--;
            argv++;
            set_log_level (LOG_INFO|LOG_VERBOSE);
        }
        else if (!strcmp (*argv, "--"))
        {
            argc--;
            argv++;
            separator = 1;
            if (argc != 2)
            {
                show_help (0);
            }
        }
        else
        {
            if (argc == 2)
            {
                break;
            }
            show_help (0);
        }
    }

    if (argc != 2)
    {
        show_help (0);
    }

    if (!strcmp (*argv, "-c") && !separator)
    {
        printf ("Error: Can't check regular expression '-c'\n"
                "\tplease use '--' after the last check dir "
                "specification if you want to do this\n");
        exit (1);
    }

    if (read_base_exp)
    {
        read_check_files (check_dir, read_base_exp);
    }

    if (!regexp_process_definitions()) {
        log_error("Error while processing definitions.\n");
    }
    else if (!regexp_process_extensions()) {
        log_error("Error while processing extensions.\n");
    }
    else if (!vartype_finalize_type_system()) {
        log_error("Error while compiling variable types.\n");
    }
    else {
        set_log_level (LOG_VERBOSE|LOG_REGEXP);
        if (! regexp_exists (argv[0]))
        {
            res = regexp_user (argv[1], argv[0], 0, 0, 1, "command line", 1);
        }
        else
        {
            printf ("using predefined regular expression from base.exp\n");
            res = regexp_chkvar ("cmd_var", argv[1], argv[0], NULL);
        }

        if (res == OK)
        {
            printf ("'%s' matches '%s'\n", argv[0], argv[1]);
        }
    }

    regexp_done();
    var_fini_module();
    vartype_fini_module();
    package_fini_module();
    return 0;
}

void
show_help (int level)
{
    printf ("usage: regexp [-h] [-v] [-vv] [-c <check dir>] [-c <check dir>] [-c ...] regexp string\n\n"
            "-h\tshow help for regular expressions\n"
            "-v\tverbose\n"
            "-vv\teven more verbose\n"
            "-c\tdirectory containing check files (*.exp), base.exp has to come first\n"
            "regexp\tregular expression\n"
            "string\tstring to be matched against regular expression\n\n");

    if (!level)
    {
        exit (1);
    }

#if 0 /* kristov: commented out as the character encoding is not ASCII or UTF-8 */
      /* please translate into english! */
    printf("\nRegul�re Ausdr�cke sind wie folgt definiert:\n\n"
           "Regul�rer Ausdruck: Eine oder mehrere Alternativen, getrennt durch\n"
           "'|', z.B. 'ro|rw|no'. Trifft eine der Alternativen zu, trifft der\n"
           "ganze Ausdruck zu (hier w�ren 'ro', 'rw' und 'no' g�ltige Ausdr�cke).\n\n"
           "Eine Alternative ist eine Verkettung mehrerer Teilst�cke, die einfach\n"
           "aneinandergereiht werden.\n\n"
           "Ein Teilst�ck ist ein \"Atom\", gefolgt von einem einzelnen '*', '+',\n"
           "'?' oder '{min, max}'. Die Bedeutung ist wie folgt:\n");
    printf("\t'a*' - beliebig viele a's einschlie�lich kein a\n"
           "\t'a+' - mindestens ein a, sonst beliebig viele a's\n"
           "\t'a?' - kein oder ein a\n"
           "\t'a{2,5} - zwei bis 5 a's\n"
           "\t'a{5} - 5 a's\n"
           "\t'a{2,} - mindestens 2 a's\n\n");
    printf("Ein \"Atom\" ist ein\n"
           "\t- regul�rer Ausdruck eingeschlossen in Klammern, z.B. (a|b)+\n"
           "          trifft auf eine beliebige Zeichenkette zu, die mindestens\n"
           "          ein a oder b enth�lt, sonst aber beliebig viele und in\n"
           "          beliebiger Reihenfolge\n"
           "\t- ein leeres Paar Klammern steht f�r einen \"leeren\" Ausdruck\n"
           "\t- ein Ausdruck mit eckigen Klammern '[]' (siehe weiter unten)\n");
    printf("\t- ein Punkt '.', der auf irgend ein einzelnes Zeichen zutrifft,\n"
           "          z.B. '.+' trifft auf eine beliebige Zeichenkette zu, die\n"
           "          mindestens ein Zeichen enth�lt\n"
           "\t- ein '^' steht f�r den Zeilenanfang, z.B. '^a.*' trifft auf\n"
           "          eine Zeichenkette zu, die mit einem a anf�ngt und in der\n"
           "          beliebige Zeichen folgen, 'a' oder 'adkadhashdkash'\n"
           "\t- ein '$' steht f�r das Zeilenende\n");
    printf("\t- ein '\\' gefolgt von einem der Sonderzeichen `^.[$()|*+?{\'\n"
           "          steht f�r genau das zweite Zeichen ohne seine spezielle Bedeutung\n"
           "\t- ein normales Zeichen trifft auf genau das Zeichen zu,\n"
           "          z.B. 'a' trifft auf genau 'a' zu.\n\n"
           "Ein Ausdruck mit rechteckigen Klammern bedeutet folgendes\n");
    printf("\t'[x-y]' - trifft auf irgend ein Zeichen zu, das zwischen\n"
           "                  x und y liegt, z.B. '[0-9]' steht f�r alle Zeichen\n"
           "                  zwischen 0 und 9; '[a-zA-Z]' f�r alle Buchstaben,\n"
           "                  egal ob gro� oder klein\n"
           "\t'[^x-y]' - trifft auf irgendein Zeichen zu, das nicht im\n"
           "                  angegebenen Intervall liegt\n");
    printf("\t'[:character_class:] - trifft auf ein Zeichen der Zeichen-Klasse\n"
           "                  zu. Relevante Standardzeichenklassen sind: alnum, alpha,\n"
           "                  blank, digit, lower, print, punct, space, upper, xdigit.\n\n"
           "Sehen wir uns das mal an einigen Beispielen an:\n\n"
           "Numerisch: Ein numerischer Wert besteht aus mindestens einer, aber\n"
           "beliebig vielen Zahlen. Mindestens ein, aber beliebig viele dr�ckt man\n"
           "mit '+' aus, eine Zahl hatten wir schon als Beispiel. Zusammengesetzt\n"
           "ergibt das:\n\n");
    printf("\tNUMERIC = '[0-9]+' oder alternativ\n"
           "\tNUMERIC = '[[:digit:]]+'\n\n"
           "NOBLANK: Ein Wert, der keine Leerzeichen enth�lt ist ein beliebiges\n"
           "Zeichen (au�er dem Leerzeichen) und davon beliebig viele:\n\n"
           "\tNOBLANK = '[^ ]*'\n\n"
           "bzw. wenn der Wert zus�tzlich auch nicht leer sein soll:\n\n"
           "\tNOBLANK = '[^ ]+'\n\n");
    printf("Disk und Partition: G�ltige Bezeichner f�r eine Disk beginnen mit hd\n"
           "bei IDE-Disks bzw sd bei SCS-Disks. Dann folgen Buchstaben von a-z (a\n"
           "f�r die erste Disk, b f�r die 2., ...) und bei Partitionen die Zahlen\n"
           "1-8 (1-4 f�r die ersten 4 Partitionen, die Prim�r bzw. Extended (nur\n"
           "eine) sein k�nnen und 5-8 f�r die logischen Partitionen innerhalb\n"
           "einer extended Partition). Die Ausdr�cke sehen dann wie folgt aus:\n\n"
           "\tDISK      = '(hd|sd)[a-z]'\n"
           "\tPARTITION = '(hd|sd)[a-z][1-8]'\n\n");
    printf("Sehen wir uns das ganze nochmal am Beispiel der IP-Addresse an. Eine\n"
           "IP-Adresse besteht aus 4 Octets, die mit einem '.' getrennt sind. Ein\n"
           "Octet kann eine Zahl zwischen 0 und 255 sein. Definieren wir als\n"
           "erstes ein Octet: Es kann\n\n"
           "\t- eine Zahl zwischen 0 und 9 sein:        [0-9]\n"
           "\t- eine Zahl zwischen 00 und 99:      [0-9][0-9]\n"
           "\t- eine Zahl zwischen 100 und 199:   1[0-9][0-9]\n"
           "\t- eine Zahl zwischen 200 und 249:   2[0-4][0-9]\n"
           "\t- eine Zahl zwischen 250 und 255 sein:  25[0-5]\n\n");
    printf("Da sich die ersten drei Teile stark �hneln, kann man sie zusammenfassen:\n\n"
           "\t- ein Zahl zwischen 0 und 199:    1?[0-9]?[0-9] (die ersten\n"
           "          beiden Stellen k�nnen, aber m�ssen nicht da sein)\n\n"
           "Das ganze sind Alternativen, also fassen wir sie einfach mittels '|' zu\n"
           "einem Ausdruck zusammen: '1?[0-9]?[0-9]|2[0-4][0-9]|25[0-5]' und haben\n"
           "damit ein Octet. Daraus k�nnen wir nun eine IP-Adresse machen, 4\n"
           "Octets mit Punkten voneinander getrennt (der Punkt mu� mittels '\\'\n");
    printf("gequotet werden, da er sonst f�r ein beliebiges Zeichen\n"
           "steht). Basierend auf der Syntax der Exp-Files sieht das ganze dann\n"
           "wie folgt aus:\n\n"
           "\tOCTET  = '1?[0-9]?[0-9]|2[0-4][0-9]|25[0-5]'\n"
           "\tIPADDR = '((RE:OCTET)\\.){3}(RE:OCTET)'\n\n");
#endif
    exit (1);
}
