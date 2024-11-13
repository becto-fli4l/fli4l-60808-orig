/*----------------------------------------------------------------------------
 *  log.c - functions for logging
 *
 *  Copyright (c) 2002 - Frank Meyer
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "libmkfli4l/log.h"
#include "libmkfli4l/str.h"

static FILE *logfp;
static int   log_level = 0;
static int   log_indent_lvl;
static char  printable_buf[32];
static char  output[4096*4];
static char  reformat_buffer[4096*4];

static char const indentation[] = "    ";
static int const  indentation_len = sizeof(indentation) - 1;

BOOL no_reformat = FALSE;

#define CHAR_TABLE_SIZE 32
static char const *char_table[CHAR_TABLE_SIZE] = {
    "\\0",
    0,
    0,
    0,
    0,
    0,
    0,
    "\\a",
    "\\b",
    "\\t",
    "\\n",
    "\\v",
    "\\f",
    "\\r",
};

char const *
log_get_printable(char c)
{
    if (isprint((int) c)) {
        printable_buf[0] = c;
        printable_buf[1] = '\0';
    }
    else {
        char const *s = c >= 0 && c < CHAR_TABLE_SIZE ? char_table[(unsigned) c] : NULL;
        if (s) {
            sprintf(printable_buf, "%s", s);
        }
        else {
            sprintf(printable_buf, "0x%02X", (unsigned char) c);
        }
    }
    return printable_buf;
}

static int
indent_output(char *s, size_t *size)
{
    int i;
    int appended = 0;
    for (i = 0; i < log_indent_lvl; ++i, s += indentation_len) {
        if ((size_t) indentation_len <= *size) {
            strcpy(s, indentation);
            appended += indentation_len;
            *size -= indentation_len;
        }
        else {
            strncpy(s, indentation, *size);
            appended += *size;
            *size = 0;
            break;
        }
    }
    return appended;
}

static char *
reformat(char *src, BOOL indent)
{
    char *tmp;
    int chars_in_line;
    int num_indent;

    char *dest = reformat_buffer;
    size_t size = sizeof(reformat_buffer) - 1;

    if (no_reformat) {
        if (indent) {
            dest += indent_output(dest, &size);
        }
        strncpy(dest, src, size);
        dest[size] = '\0';
        return reformat_buffer;
    }

    tmp = strchr(src, ':');
    if (tmp && tmp - src < 15) {
        num_indent = tmp - src + 2;
    }
    else {
        num_indent = indentation_len;
    }

    chars_in_line = 0;
    while (*src && size > 0) {
        BOOL first_word;
        *dest = '\0';

        /* indent text */
        if (indent) {
            int const c = indent_output(dest, &size);
            dest += c;
            chars_in_line = c;
        }

        if (size == 0) {
            break;
        }

        first_word = TRUE;
        while (size > 0) {
            int len;

            /* find next word boundary */
            for (tmp = src; *tmp && !isspace((int) *tmp); ++tmp) {
                ;
            }
            len = tmp - src;
            if (len == 0 && !*tmp) {
                break;
            }

            if (chars_in_line + len > 70 && !first_word) {
                int i;
                int max;
                *dest++ = '\n';
                --size;
                max = MIN((size_t) num_indent, size);

                for (i = 0; i < max; ++i)
                    *dest++ = ' ';
                size -= max;
                chars_in_line = num_indent;
                break;
            }

            first_word = FALSE;
            chars_in_line += len + 1;
            while (src <= tmp && size > 0) {
                *dest++ = *src++;
                --size;
            }
        }
    }
    *dest = '\0';

    return reformat_buffer;
}

static void
do_log_buffer(char *buffer, BOOL do_reformat)
{
    char *const real_output = reformat(buffer, do_reformat);
    fprintf(stderr, "%s", real_output);

    if (logfp) {
        fprintf(logfp, "%s", real_output);
    }
}

static void
do_log(const char *fmt, va_list ap, BOOL do_reformat)
{
    vsprintf(output, fmt, ap);
    do_log_buffer(output, do_reformat);
}

void
open_logfile(char const *logfile)
{
    close_logfile();
    logfp = fopen(logfile, "w");
    if (!logfp) {
        log_error("Error opening log file '%s': %s\n",
                  logfile, strerror(errno));
        abort();
    }
}

void close_logfile(void)
{
    if (logfp) {
        fclose (logfp);
        logfp = 0;
    }
}

void
set_log_level(int level)
{
    log_level = level;
}

void
inc_log_indent_level(void)
{
    ++log_indent_lvl;
}

void
dec_log_indent_level(void)
{
    if (log_indent_lvl > 0) {
        --log_indent_lvl;
    }
}

void
log_lex_fprintf(FILE *file, char const *fmt, ...)
{
    static char const useless[] = "--(end of buffer";
    va_list ap;
    char        lex_msg_buf[1024];
    char        msg_buf[1024];
    const char *p;
    char       *q;

    if (!strncmp(fmt, useless, sizeof(useless) - 1)) {
        return;
    }

    va_start(ap, fmt);
    vsprintf(lex_msg_buf, fmt, ap);
    va_end(ap);

    p = lex_msg_buf;
    q = msg_buf;

    while (*p) {
        if (isprint((int) *p) || !*(p + 1)) {
            *q++ = *p++;
            *q = '\0';
        }
        else {
            char const *n = log_get_printable(*p);
            strcat(q, n);
            q += strlen(n);
            *q = '\0';
            ++p;
        }
    }
    *q = '\0';

    do_log_buffer(msg_buf, TRUE);
    (void) file;
}

void
log_yacc_fprintf(FILE * file, char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_log(fmt, ap, TRUE);
    va_end(ap);
    (void) file;
}


void
log_info(int level, char const *fmt, ...)
{
    va_list ap;

    if (log_level & level) {
        va_start(ap, fmt);
        do_log(fmt, ap, TRUE);
        va_end(ap);
    }
}

void
log_warn(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_log(fmt, ap, TRUE);
    va_end(ap);
}

void
log_error(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_log(fmt, ap, FALSE);
    va_end(ap);
}

void
fatal_exit(char const *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_log(fmt, ap, FALSE);
    va_end(ap);

    close_logfile();
    if (log_level) {
        fprintf(stderr, "creating core dump (or stack trace under windows) "
                "for debugging purposes...\n");
        abort();
    }
    else {
        exit(1);
    }
}
