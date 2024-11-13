/*----------------------------------------------------------------------------
 *  squeeze_main.c - squeezes shell scripts
 *
 *  Copyright (c) 2005-2016 - fli4l-Team <team@fli4l.de>
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
#include <stdlib.h>

extern int squeezelex(void);
extern int squeeze_lex_init(FILE * in, FILE * out);

int main (int argc, char ** argv)
{
         FILE * in = NULL, * out = NULL;

         argc--; argv++;
#ifdef FLEX_DEBUG
         if (argc > 0 && !strcmp(argv[0], "-d"))
         {
             yy_flex_debug = 1;
             argv++;
             argc--;
         }
         else
             yy_flex_debug = 0;
#endif
         if (argc > 0)
         {
             in = fopen (argv[0], "r");
             if (!in)
             {
                  fprintf(stderr, "unable to open '%s' for reading, aborting\n", argv[0]);
                  exit(1);
             }
             argv++;
             argc--;
         }
         if (argc > 0)
         {
             out = fopen (argv[0], "wb");
             if (!out)
             {
                  fprintf(stderr, "unable to open '%s' for writing, aborting\n", argv[0]);
                  exit(1);
             }
         }
         squeeze_lex_init(in, out);
         return squeezelex();
}
