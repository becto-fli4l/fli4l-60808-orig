/*----------------------------------------------------------------------------
 *  mk_syslinux.c - functions for creating syslinux.cfg file
 *
 *  Copyright (c) 2003 - Gernot Miksch
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
 *  Creation:    2003-02-27  gm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <libmkfli4l/check.h>
#include <libmkfli4l/var.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/parse.h>
#include <libmkfli4l/str.h>
#include "mk_syslinux.h"

int
mk_syslinux (char * fname_s,char * fname_d)
{
    char        buf[256];
    char const *p, *p1;
    FILE *      fp_s;
    FILE *      fp_d;
    char        console[40];
    int         line,error;
    int         debug_startup = 0;
    char const *debug_log = NULL;
    char const *boot_param = "";
    char const *ext_boot_param = "";
    char const *tag_str = "TAG";
    char timeout_str[20] = "TIMEOUT 50";
    error=OK;

    console[0]='\0';
    fp_s = fopen (fname_s, "r"); /* open template-file */
    if (! fp_s)
    {
        log_error ("Error while opening template file '%s': %s\n",
                   fname_s, strerror (errno));
        error=ERR_MKSYSL_TMPL;
    }
    else
    {
        fp_d = fopen (fname_d, "wb"); /* open destination-file */
        if (! fp_d)
        {
            log_error ("Error while opening destination file '%s': %s\n",
                       fname_d, strerror (errno));
            error=ERR_MKSYSL_CFG;
        }
        else
        {
            int onlyserial=0;
            int secondary=0;
            int primary=0;
            p = get_variable ("SER_CONSOLE");
            if (p && ( (onlyserial = !strcmp (p, "yes"))       ||
                       (secondary  = !strcmp (p, "secondary")) ||
                       (primary    = !strcmp (p, "primary")) ))
            {
                char const *arch;
                char const *ifname;
                arch = get_variable ("ARCH");
                p = get_variable ("SER_CONSOLE_IF");
                p1 = get_variable ("SER_CONSOLE_RATE");
                ifname = get_variable ("SER_CONSOLE_IF_NAME");

                fprintf (fp_d, "SERIAL %s %s\n", p, p1);
                /* Disable output to the normal video console when
                   serial console is enabled only. Some BIOSes try
                   to forward this to the serial console and sometimes
                   make a total mess. */
                if (onlyserial) {
                    fprintf (fp_d, "CONSOLE 0\n");
                }
                *console = '\0';
                if (primary)
                {
                    strcpy (console, "console=tty0 ");
                }
                if (ifname) {
                    sprintf (console+strlen (console),
                             "console=%s,%sn8", ifname, p1);
                } else {
                    sprintf (console+strlen (console),
                             "console=ttyS%s,%sn8", p, p1);
                    if (arch && strcmp(arch, "x86") != 0 && strcmp(arch, "x86_64") != 0) {
                        log_warn("Using hard-coded mapping from serial console %s to device ttyS%s which may fail on non-x86 architectures; use package 'hwsupp' instead!\n",
                                 p, p);
                    }
                }
                if (secondary)
                {
                    strcat (console, " console=tty0");
                }
            }

            p = get_variable ("DEBUG_STARTUP");
            if (p && ! strcmp (p, "yes"))
            {
                debug_startup = 1;
                p = get_variable ("DEBUG_LOG");
                if (p && *p)
                {
                    debug_log = p;
                }
            }

            p = get_variable ("KERNEL_BOOT_OPTION");
            if (p && *p)
            {
                boot_param = p;
            }

            p = get_variable ("KERNEL_BOOT_OPTION_EXT");
            if (p)
            {
                while (isspace((int) *p)) p++;
                if (*p) ext_boot_param = p;
            }

            p = get_variable ("BOOTMENU_TIME");
            if (p && *p)
            {
                p1 = get_variable ("BOOT_TYPE");
                if (p1 && ! strcmp (p1, "hd"))
                {
                    sprintf (timeout_str, "TIMEOUT %s0", p);
                }
                else
                {
                    sprintf (timeout_str, "TIMEOUT 1");
                }
            }

            error=ERR_MKSYSL_APPEND;
            for (line = 1; fgets (buf, 256, fp_s); line++)
            {
                char *p;
                if (buf[0] == '#'&&buf[1] == '#')
                {
                    /* double-comment: '##'     don't copy it! */
                    continue;
                }

                p = strchr (buf, '\r'); /* unix: we read a DOS file */
                if (p)
                {
                    *p = '\0';
                }
                else
                {
                    p = strchr (buf, '\n');
                    if (p)
                    {
                        *p = '\0';
                    }
                }
                p=strstr(buf,"TIMEOUT");
                if (p)
                {
                    *p = '\0';
                    strcat (buf, timeout_str);
                }

                p=strstr(buf,"APPEND");
                if (p)
                {
                    error=OK;
                    p=strstr(buf, tag_str);
                    if (p)
                    {
                        *p = '\0';
                        strcat (buf, console);
                        if (strlen (boot_param) > 0)
                        {
                            strcat (buf, " ");
                            strcat (buf, boot_param);
                        }
                        if (strlen (ext_boot_param) > 0)
                        {
                            strcat (buf, " ");
                            strcat (buf, ext_boot_param);
                        }
                        if (debug_startup)
                        {
                            strcat (buf, " fli4ldebug=yes");
                            if (debug_log)
                            {
                                strcat (buf, " fli4ldeblog=");
                                strcat (buf, debug_log);
                            }
                        }
                    }
                }
                fprintf(fp_d,"%s\n",buf);
            }

            if (error==ERR_MKSYSL_APPEND)
            {
                log_error ("Error: no APPEND-line found in syslinux-template\n");
            }
            fclose(fp_d);
        }
        fclose(fp_s);
    }
    return(error);
}
