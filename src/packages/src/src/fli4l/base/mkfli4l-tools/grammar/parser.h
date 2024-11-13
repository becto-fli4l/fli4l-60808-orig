/*****************************************************************************
 *  @file grammar/parser.h
 *  Common definitions used by all parsers.
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

#include "libmkfli4l/log.h"

#define PARSER_DEFINE_ERROR_PROLOGUE(treetype) \
    static void \
    yyerror( \
        YYLTYPE *loc, \
        yyscan_t scanner, \
        struct package_file_t *file, \
        treetype tree,

#define PARSER_DEFINE_ERROR_EPILOGUE(text) \
        char const *msg \
    ) \
    { \
        struct location_t *location \
            = location_create(file, loc->first_line, loc->first_column); \
        char *locstr = location_toString(location); \
        log_error("Error while parsing " text " at %s: %s\n", locstr, msg); \
        free(locstr); \
        location_destroy(location); \
    }
