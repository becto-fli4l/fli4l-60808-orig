/*----------------------------------------------------------------------------
 *  cfg.l - functions for scanning configuration files
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

%option noinput
%option nounput
%option noyy_top_state
%option noyywrap
%option never-interactive

%{
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "libmkfli4l/defs.h"
#include "libmkfli4l/cfg.h"
#include "libmkfli4l/str.h"
#include "libmkfli4l/log.h"

#define MAX_STR_CONST (4096 * 2)
static char string_buf[MAX_STR_CONST + 1];
static char *string_buf_ptr = NULL;
static BOOL quote_active = FALSE;
static BOOL multi_flag = FALSE;

static char *cfgfile = NULL;
static int cfgline = 0;
static int cfgcolumn = 0;
struct cfglval_t cfglval;

#define fprintf log_lex_fprintf

static void
cfg_new_line(void)
{
    ++cfgline;
    cfgcolumn = 0;
    log_info(LOG_SCAN, "cfgline: %d\n", cfgline);
}

static void
copy_string(char const *str, int length)
{
    int i;
    char const *p;

    for (i = 0, p = str; i < length && *p; ++i, ++p) {
        if (iscntrl((int) *p) && *p != '\r' && *p != '\t') {
            fatal_exit("%s:%d : Control character 0x%02X found in string, "
                       "aborting\n",
                       cfgfile, cfgline, (unsigned char) *p);
        }
    }

    if (length < &string_buf[MAX_STR_CONST-1] - string_buf_ptr) {
        strncpy(string_buf_ptr, str, length);
        string_buf_ptr += length;
    }
    else {
        fatal_exit("string too long ...");
    }
}
%}

%x sq_str dq_str

BOM             \xEF\xBB\xBF
ID              [A-Z][A-Z0-9_%]*
NEG_ID          !{ID}
OPT_ID          \+{ID}
REALLY_OPT_ID   \+\+{ID}
DEP_ID          \+?{ID}\({ID}(=~\'[^\']*\')?\)
STR_ELEM        \\\\|\\\'|[^'\n]
ML_STR_ELEM     \\\\|\\\'|[^']
STRING          \'{STR_ELEM}*\'
ML_STRING       \'({ML_STR_ELEM}*[\n])*\'
REGEXP          RE:[^[:space:]]+
DCOMMENT        ^{BOM}?{SPACE}*[#]{1,2}[^\n\r]*
COMMENT         [ \t\r]+[#][^\n\r]*
WCOMMENT        [#][^\n\r]*
NL              [\n]
HYPHEN          [-]
EQUAL           [=]
COLON           [:]
SPACE           [ ]+
CR              [\r]
TAB             [\t]
%%

\'              {
                        string_buf_ptr = string_buf;
                        multi_flag = FALSE;
                        cfglval.dq = 0;
                        quote_active = TRUE;
                        BEGIN(sq_str);
                }
<sq_str>\'      {
                        /* saw closing quote - all done */
                        BEGIN(INITIAL);
                        quote_active = FALSE;
                        *string_buf_ptr = '\0';
                        log_info(LOG_SCAN, "found single quoted string '%s' (%s)\n",
                                 string_buf,
                                 multi_flag ? "multi line string" : "normal string");
                        cfglval.text = strsave(string_buf);
                        return multi_flag ? CFG_ML_STRING : CFG_STRING;
                }

\"              {
                        string_buf_ptr = string_buf;
                        multi_flag = FALSE;
                        cfglval.dq = 1;
                        quote_active = TRUE;
                        BEGIN(dq_str);
                }
<dq_str>\"      {
                        /* saw closing quote - all done */
                        BEGIN(INITIAL);
                        quote_active = FALSE;
                        *string_buf_ptr = '\0';
                        log_info(LOG_SCAN, "found double quoted string '%s' (%s)\n",
                                 string_buf,
                                 multi_flag ? "multi line string" : "normal string");
                        cfglval.text = strsave(string_buf);
                        return multi_flag ? CFG_ML_STRING : CFG_STRING;
                }

<sq_str,dq_str>[\n]     {
                        multi_flag = TRUE;
                        cfg_new_line();
                }

<sq_str>[^'\n]+ {
                        copy_string(cfgtext, cfgleng);
                }
<dq_str>[^"\\\n]+       {
                        copy_string(cfgtext, cfgleng);
                }
<dq_str>\\.     {
                        copy_string(cfgtext, cfgleng);
                }
<dq_str,sq_str><<EOF>>  {
                        log_error("Error in %s:%d,%d: expecting closing single/double quote, got EOF\n",
                                  cfgfile, cfgline, cfgcolumn);
                        BEGIN(INITIAL);
                        yyterminate();
                }

{BOM}

{ID}            {
                        cfglval.text = strsave(cfgtext);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_ID;
                }
{NEG_ID}        {
                        cfglval.text = strsave(cfgtext+1);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_NEG_ID;
                }
{OPT_ID}        {
                        cfglval.text = strsave(cfgtext+1);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_OPT_ID;
                }
{DEP_ID}        {
                        cfglval.text = strsave(cfgtext);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_DEP_ID;
                }
{REALLY_OPT_ID} {
                        cfglval.text = strsave(cfgtext+2);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_REALLY_OPT_ID;
                }
{REGEXP}        {
                        cfglval.text = strsave(cfgtext+3);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_REGEXP;
                }
{HYPHEN}        {
                        cfglval.text = NULL;
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_HYPHEN;
                }
{EQUAL}         {
                        cfglval.text = NULL;
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_EQUAL;
                }
{COLON}         {
                        cfglval.text = NULL;
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_COLON;
                }
{DCOMMENT}

{COMMENT}       {
                        cfglval.text = strsave(cfgtext);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_COMMENT;
                }

{WCOMMENT}      {
                        cfglval.text = strsave(cfgtext);
                        cfglval.column = cfgcolumn;
                        cfgcolumn += cfgleng;
                        return CFG_WCOMMENT;
                }


{SPACE}         { cfgcolumn += cfgleng; }
{TAB}           { cfgcolumn += 8; }
{CR}

{NL}            {
                        cfglval.text = NULL;
                        cfglval.column = cfgcolumn;
                        cfg_new_line();
                        return CFG_NL;
                }

.               {
                        cfglval.text = NULL;
                        cfglval.column = cfgcolumn;
                        return CFG_UNKNOWN;
                }
%%

static FILE *cfgfp = NULL;

static char const *token_names[] = {
    "conditional identifier (depends on other identifier)",
    "identifier",
    "optional identifier (identifier with leading '+')",
    "really optional identifier (identifier with leading '++')",
    "string (enclosed in \"'\")",
    "string (maybe stretched across several lines)",
    "regular expression (starting with 'RE:')",
    "comment (starting with some spaces followed by '#')",
    "wrong comment (starting with '#' without leading spaces)",
    "=",
    ":",
    "-",
    "new line",
    "unknown character (e.g. lower case character)",
};

static void
cfg_error(int expected, int got, int column)
{
    int i;
    BOOL first = TRUE;
    char const *gotstr = NULL;
    log_error("Error in %s:%d,%d: expected ", cfgfile, cfgline, column);
    for (i = 0; i < 32; ++i) {
        if (expected & (1<<i)) {
            if (first) {
                first = FALSE;
            }
            else {
                log_error(" or ");
            }
            log_error("\"%s\"", token_names[i]);
        }
        if (got & (1<<i)) {
            gotstr = token_names[i];
        }
    }
    log_error(", got '%s' (%x)\n", gotstr, got);
}

void
cfg_fopen(char const *file)
{
    free(cfgfile);
    cfgfile = strsave(file);

    cfgfp = fopen(file, "r");
    if (!cfgfp) {
        fatal_exit("Error opening file '%s': %s\n",
                   file, strerror(errno));
    }

    cfgrestart(cfgfp);
    cfgline = 1;
    cfgcolumn = 1;
    log_info(LOG_SCAN, "cfgestart, reading \"%s\"\n", file);
}

void
cfg_fclose(void)
{
    fclose(cfgfp);
}

int
get_config_tokens(struct token_t *t, int ignore_nl)
{
    int ret;
    BOOL first_token = TRUE;
    BOOL eof = FALSE;

    log_info(LOG_SCAN, "entering get_config_tokens\n");

    while (t->allowed_tokens != CFG_NONE) {
        log_info(LOG_SCAN, "calling cfglex \n");
        ret = cfglex();
        if (first_token) {
            if (!ret) {
                return CFG_EOF;
            }
            else if (ret == CFG_NL || ret == CFG_COMMENT) {
                continue;
            }
            else {
                first_token = FALSE;
            }
        }
        else if (!ret) {
            if (quote_active || eof) {
                log_error("Error in %s:%d,%d: unexpected end of file\n",
                          cfgfile, cfgline, cfglval.column);
                return CFG_ERROR;
            }
            eof = TRUE;
            ret = CFG_NL;
        }

        log_info(LOG_SCAN, "cfglex returned with 0x%x \n", ret);
        if (ret & t->allowed_tokens) {
            char const *p;

            t->token = ret;
            t->text  = cfglval.text;
            t->dq = cfglval.dq;
            t->line = cfgline;

            switch (ret) {
            case CFG_NL:
                return 1;
            case CFG_HYPHEN:
                p = "-";
                break;
            default:
                p = cfglval.text ? cfglval.text : "unknown";
                break;
            }
            log_info(LOG_SCAN, "got 0x%x ('%s')\n", t->token, p);
        }
        else {
            if (ignore_nl && ret == CFG_NL) {
                continue;
            }

            cfg_error(t->allowed_tokens, ret, cfglval.column);
            /* skip line */
            while (ret && ret != CFG_NL) {
                ret = cfglex();
            }
            return CFG_ERROR;
        }
        ++t;
    }

    log_info(LOG_SCAN, "leaving get_config_tokens\n");
    return 0;
}
