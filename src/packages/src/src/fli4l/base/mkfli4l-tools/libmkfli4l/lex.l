/*----------------------------------------------------------------------------
 *  lex.l - functions for scanning extended check scripts
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

%option noinput
%option nounput
%option noyy_top_state
%option noyywrap
%option never-interactive

%{
#include "libmkfli4l/defs.h"
#include "libmkfli4l/tree.h"
#include "libmkfli4l/grammar.h"
#include "libmkfli4l/log.h"
#include <ctype.h>

int yyline = 1;

#define fprintf log_lex_fprintf
%}

BOM             \xEF\xBB\xBF
ID              [a-zA-Z][a-zA-Z0-9_]*
IDSET           [a-zA-Z]((_%_)?[a-zA-Z0-9_]*)*(_%)?
IF              if|IF
THEN            then|THEN
ELSE            else|ELSE
FI              fi|FI
DEFINED         defined|DEFINED
WARNING         warning|WARNING
ERROR           error|ERROR
FATAL_ERROR     fatal_error|FATAL_ERROR
ADD_TO_OPT      add_to_opt|ADD_TO_OPT
FGREP           fgrep|FGREP
STAT            stat|STAT
SPLIT           split|SPLIT
CRYPT           crypt|CRYPT
NUMERIC         numeric|NUMERIC
COPY_PENDING    copy_pending|COPY_PENDING
UNIQUE          unique|UNIQUE
SET             set|SET
PROVIDES        provides|PROVIDES
DEPENDS         depends|DEPENDS
ON              on|ON
FOREACH         foreach|FOREACH
IN              in|IN
DO              do|DO
DONE            done|DONE
SAMENET         samenet|SAMENET
SUBNET          subnet|SUBNET
VERSION         version|VERSION
GREATER         greater|GREATER
VER             [0-9]+\.[0-9]+(\.[0-9]+)?
STRING          \"[^\"]*\"
CHAR            '.'
%%

{BOM}
(0x)?[0-9]+     { yylval.elem = mkleaf(NUM, yytext); return NUM; }
{IF}            { yylval.line = yyline; return IF; }
{THEN}          { yylval.line = yyline; return THEN; }
{ELSE}          { yylval.line = yyline; return ELSE; }
{FI}            { return FI; }
{SET}           { yylval.line = yyline; return SET; }
{WARNING}       { yylval.line = yyline; return WARNING; }
{ERROR}         { yylval.line = yyline; return ERROR; }
{FATAL_ERROR}   { yylval.line = yyline; return FATAL_ERROR; }
{ADD_TO_OPT}    { yylval.line = yyline; return ADD_TO_OPT; }
{FGREP}         { yylval.line = yyline; return FGREP; }
{STAT}          { yylval.line = yyline; return STAT; }
{CRYPT}         { yylval.line = yyline; return CRYPT; }
{SPLIT}         { yylval.line = yyline; return SPLIT; }
{NUMERIC}       { yylval.line = yyline; return NUMERIC; }
{COPY_PENDING}  { return COPY_PENDING; }
{UNIQUE}        { return UNIQUE; }
{DEFINED}       { return DEFINED; }
{STRING}        { yylval.elem = mkleaf(STRING, yytext); return STRING; }
{CHAR}          { yylval.elem = mkleaf(CHAR, yytext); return CHAR; }
{PROVIDES}      { yylval.line = yyline; return PROVIDES; }
{VERSION}       { return VERSION; }
{DEPENDS}       { yylval.line = yyline; return DEPENDS; }
{ON}            { return ON; }
{FOREACH}       { yylval.line = yyline; return FOREACH; }
{IN}            { return IN; }
{DO}            { return DO; }
{DONE}          { return DONE; }
{SAMENET}       { return SAMENET; }
{SUBNET}        { return SUBNET; }
{VER}           { yylval.elem = mkleaf(VER, yytext); return VER; }
{ID}            { yylval.elem = mkleaf(ID, yytext); return ID; }
{IDSET}         { yylval.elem = mkleaf(IDSET, yytext); return IDSET; }
"=="            { return EQUAL; }
"!="            { return NOT_EQUAL; }
"=~"            { return MATCH; }
"="             { return ASSIGN; }
"("             { return '('; }
")"             { return ')'; }
"["             { return '['; }
"]"             { return ']'; }
","             { return ','; }
"<"             { return LESS; }
"<="            { return LE; }
">="            { return GE; }
">"             { return GREATER; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
"+"             { return ADD; }
"-"             { return SUB; }
"*"             { return MULT; }
"/"             { return DIV; }
"%"             { return MOD; }
"?:"            { return ORELSE; }
":"             { return CAST; }

"#".*           /* eat up one-line comments */

[ \r\t]+        /* eat up whitespace */
[\n]+           { yyline += yyleng; } /* eat up new lines */

.               { return UNKNOWN; }

%%
