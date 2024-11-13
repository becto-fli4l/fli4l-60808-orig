/*****************************************************************************
 *  @file grammar/scanner.h
 *  Common definitions used by all scanners.
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

#define SCANNER_TAB_SIZE 8
#define SCANNER_BOM "\xEF\xBB\xBF"
#define SCANNER_BOM_LEN ((int) (sizeof SCANNER_BOM - 1))

#define YY_USER_ACTION \
{ \
    char const *p = yytext; \
    int len = yyleng; \
    int last_line = yylineno; \
    int last_column = yycolumn - 1; \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (yyleng >= SCANNER_BOM_LEN \
            && strncmp(p, SCANNER_BOM, SCANNER_BOM_LEN) == 0) { \
        p += SCANNER_BOM_LEN; \
        len -= SCANNER_BOM_LEN; \
    } \
    while (len > 0) { \
        if (*p == '\t') { \
            last_column = yycolumn; \
            last_line = yylineno; \
            yycolumn += SCANNER_TAB_SIZE; \
            yycolumn -= yycolumn % SCANNER_TAB_SIZE; \
        } \
        else if (*p == '\r') { \
            ; \
        } \
        else if (*p == '\n') { \
            last_column = yycolumn; \
            last_line = yylineno; \
            ++yylineno; \
            yycolumn = 0; \
        } \
        else { \
            last_column = yycolumn; \
            last_line = yylineno; \
            ++yycolumn; \
        } \
        ++p; \
        --len; \
    } \
    yylloc->last_line = last_line; \
    yylloc->last_column = last_column; \
}
