/*----------------------------------------------------------------------------
 *  buildinfo.c - functions for generating build information
 *
 *  Copyright (c) 2012-2016 - fli4l-Team <team@fli4l.de>
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/options.h>
#include "buildinfo.h"
#include "var.h"

int
set_arch(void)
{
    FILE *f;
    int rc = OK;

    f = fopen("arch.txt", "r");
    if (f != NULL) {
        char line[BUF_SIZE];
        if (fgets(line, sizeof line - 1, f) != NULL) {
            char *blank = strchr(line, '\n');
            if (blank) {
                *blank = '\0';
            }

            var_write("ARCH", line, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);
        }
        else {
            log_error("Cannot read arch.txt\n");
            rc = ERR;
        }

        fclose(f);
    }
    else {
        log_error("Cannot open arch.txt for reading\n");
        rc = ERR;
    }

    return rc;
}

int
create_build_info(void)
{
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char datestr[12];
    char timestr[10];
    FILE *f;
    int rc = OK;

    sprintf(datestr, "%d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    sprintf(timestr, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    var_write("FLI4L_BUILDDIR", config_dir, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);
    var_write("FLI4L_BUILDDATE", datestr, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);
    var_write("FLI4L_BUILDTIME", timestr, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);

    f = fopen("version.txt", "r");
    if (f != NULL) {
        char line[BUF_SIZE];
        if (fgets(line, sizeof line - 1, f) != NULL) {
            char *blank = strchr(line, '\n');
            if (blank) {
                *blank = '\0';
            }
            var_write("FLI4L_VERSION", line, NULL, VAR_ACCESS_STRONG, VAR_PRIORITY_CONFIGURATION, FALSE);
        }
        else {
            log_error("Cannot read version.txt\n");
            rc = ERR;
        }

        fclose(f);
    }
    else {
        log_error("Cannot open version.txt for reading\n");
        rc = ERR;
    }

    return rc;
}
