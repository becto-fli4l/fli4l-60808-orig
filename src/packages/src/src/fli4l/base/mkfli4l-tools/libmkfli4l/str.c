/*----------------------------------------------------------------------------
 *  str.c - functions for string processing
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "libmkfli4l/defs.h"
#include "libmkfli4l/var.h"
#include "libmkfli4l/log.h"
#include "libmkfli4l/str.h"

char *
convert_to_upper(char const *str)
{
    static char         *rtc;
    unsigned char const *s;
    unsigned char       *r;

    if (!str)
        return 0;

    if (rtc == str) {
        fatal_exit("%s %d: freeing string still in use\n", __FILE__, __LINE__);
    }

    if (rtc)
        free(rtc);

    rtc = strsave(str);
    if (!rtc)
        return 0;

    for (s = (unsigned char const *) str, r = (unsigned char *) rtc; *s; ++s, ++r)
    {
        if (islower(*s))
            *r = toupper(*s);
        else
            *r = *s;
    }

    *r = '\0';
    return rtc;
}

char *
strsave(char const *s)
{
    return s == 0 ? 0 : strdup(s);
}

char *
substrsave(char const *start, char const *end)
{
    int const len = end - start;
    char *const result = (char *) safe_malloc(len + 1);
    strncpy(result, start, len);
    result[len] = '\0';
    return result;
}

char *
strcat_save(char *s, char const *a)
{
    int len, len_s;

    if (!a)
        return s;

    len_s = s ? strlen(s) : 0;
    len   = len_s + strlen(a);

    s = safe_realloc(s, len + 1);
    strcpy(s + len_s, a);
    return s;
}

char *
strsave_quoted(char const *s)
{
    char *t;
    int   len;

    if (s && (len = strlen(s)) > 1) {
        t = calloc(len - 1, 1);
        if (t) {
            strncpy(t, s + 1, len - 2);
            t[len - 2] = '\0';
        }
    }
    else
        t = 0;

    return t;
}

char *
strsave_ws(char const *s)
{
    char *const t = strsave(s);
    char *ret;

    if (t) {
        char *p, *q;
        for (p = q = t; *q; ++p) {
            *p = (*q == '\t') ? ' ' : *q;
            if (isspace((int) *q)) {
                while (isspace((int) *++q)) {
                }
            }
            else
                ++q;
        }
        *p = '\0';
        ret = strsave(t);
        free(t);
    } else
        ret = 0;

    return ret;
}

char *
get_set_var_name_int(char const *s, int index, char const *file, int line)
{
    char var_buf [VAR_SIZE+1];
    char fmt_buf [VAR_SIZE+1];
    char *p;
    char const *fmt;
    int  index_found = FALSE;

    if (strlen(s) >= VAR_SIZE) {
        fatal_exit ("%s:%d : %s: Variable name too long\n", file, line, s);
    }

    for (p = fmt_buf, fmt = s; *fmt; ++fmt) {
        *p++ = *fmt;

        if (*fmt == '%') {
            if (!index_found) {
                index_found = TRUE;
                *p++ = 'd';
            }
            else {
                *p++ = '%';
            }
        }
    }
    *p = '\0';

    sprintf(var_buf, fmt_buf, index);

    /* TODO: if this condition is true, a buffer overflow has already occurred */
    if (strlen(var_buf) >= VAR_SIZE) {
        fatal_exit ("%s:%d : %s: Variable name too long\n", file, line, var_buf);
    }

    return strsave (var_buf);
}

char *
get_set_var_name_string(char const *s, char const *index, char const *file, int line)
{
    return get_set_var_name_int (s, convert_to_long (index, file, line),
                                 file, line);
}

char *
strip_multiple_indices(char const *s)
{
    char *const t = strsave(s);
    char *p = strchr(t, '%');
    if (p && strchr(p + 1, '%')) {
        *++p = '\0';
    }
    return t;
}

char *
replace_set_var_indices(char const *s)
{
    char *src = strsave(s);
    char *dst = src;
    char *ret = src;

    while (*src) {
        if (isdigit((int) *src) && (src != s) && (*(src - 1) == '_')) {
            *dst++ = '%';
            while (isdigit((int) *++src)) {
            }
        }
        else
            *dst++ = *src++;
    }

    *dst = '\0';
    return ret;
}

unsigned long
convert_to_long(char const *val, char const *file, int line)
{
    char *endptr = 0;
    unsigned long res;

    if (*val == '0' && *(val + 1) == 'x') {
        res = strtoul (val, &endptr, 16);
    }
    else {
        res = strtoul (val, &endptr, 10);
    }

    if (*endptr) {
        fatal_exit ("invalid number '%s' in %s, line %d\n",
                    val, file, line);
    }

    return res;
}

void *
str_safe_malloc(size_t size, char const *file, char const *function, int line)
{
    void *const p = malloc(size);
    if (!p && size > 0) {
        fatal_exit ("%s:%d, %s: unable to allocate memory\n", file, line, function);
    }
    return p;
}

void *
str_safe_realloc(void *p, size_t size, char const *file, char const *function, int line)
{
    p = realloc(p, size);
    if (!p && size > 0) {
        fatal_exit ("%s:%d, %s: unable to reallocate memory\n", file, line, function);
    }
    return p;
}

char *
safe_sprintf(char const *fmt, ...)
{
    char *buf = NULL;
    size_t bufsize = strlen(fmt);

    while (TRUE) {
        int written;
        va_list ap;

        bufsize *= 2;
        buf = (char *) safe_realloc(buf, bufsize);
        va_start(ap, fmt);
        written = vsnprintf(buf, bufsize, fmt, ap);
        va_end(ap);

        if (written < 0) {
            free(buf);
            return NULL;
        }
        else if ((size_t) written < bufsize) {
            return buf;
        }
    }
}
