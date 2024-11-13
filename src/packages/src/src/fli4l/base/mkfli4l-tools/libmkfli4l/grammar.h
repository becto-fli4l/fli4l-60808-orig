/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_LIBMKFLI4L_GRAMMAR_H_INCLUDED
# define YY_YY_LIBMKFLI4L_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    IDSET = 259,
    NUM = 260,
    STRING = 261,
    CHAR = 262,
    VER = 263,
    IF = 264,
    THEN = 265,
    ELSE = 266,
    FOREACH = 267,
    WARNING = 268,
    ERROR = 269,
    FATAL_ERROR = 270,
    FGREP = 271,
    STAT = 272,
    CRYPT = 273,
    SPLIT = 274,
    NUMERIC = 275,
    SET = 276,
    ADD_TO_OPT = 277,
    PROVIDES = 278,
    DEPENDS = 279,
    VERSION = 280,
    UNKNOWN = 281,
    FI = 282,
    IN = 283,
    DO = 284,
    DONE = 285,
    ON = 286,
    SAMENET = 287,
    SUBNET = 288,
    ASSIGN = 289,
    ORELSE = 290,
    AND = 291,
    OR = 292,
    ADD = 293,
    SUB = 294,
    MULT = 295,
    DIV = 296,
    MOD = 297,
    EQUAL = 298,
    NOT_EQUAL = 299,
    LESS = 300,
    GREATER = 301,
    LE = 302,
    GE = 303,
    MATCH = 304,
    NOT = 305,
    COPY_PENDING = 306,
    DEFINED = 307,
    UNIQUE = 308,
    CAST = 309
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 42 "libmkfli4l/grammar.y" /* yacc.c:1913  */

    int line;
    elem_t *elem;

#line 114 "libmkfli4l/grammar.h" /* yacc.c:1913  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_LIBMKFLI4L_GRAMMAR_H_INCLUDED  */
