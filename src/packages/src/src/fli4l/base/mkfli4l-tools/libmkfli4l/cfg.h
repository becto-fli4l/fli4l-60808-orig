/*----------------------------------------------------------------------------
 *  cfg.h - functions for scanning configuration files
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

#ifndef LIBMKFLI4L_CFG_H_
#define LIBMKFLI4L_CFG_H_

#define CFG_NONE        0x0
#define CFG_ID          0x1
#define CFG_DEP_ID      0x2
#define CFG_OPT_ID      0x4
#define CFG_REALLY_OPT_ID  0x8
#define CFG_STRING      0x10
#define CFG_ML_STRING   0x20
#define CFG_REGEXP      0x40
#define CFG_COMMENT     0x80
#define CFG_WCOMMENT    0x100
#define CFG_EQUAL       0x200
#define CFG_COLON       0x400
#define CFG_HYPHEN      0x800
#define CFG_NL          0x1000
#define CFG_UNKNOWN     0x2000
#define CFG_NEG_ID      0x4000

#define CFG_EOF -2
#define CFG_ERROR -3

struct cfglval_t {
    char *text;
    int column;
    int dq;
};

struct token_t {
    int allowed_tokens;
    int token;
    int dq;
    int line;
    char *text;
};

#define TOKEN(x) { x, 0, 0, 0, "", }

extern int cfg_flex_debug;

void cfg_fopen(char const *file);
void cfg_fclose(void);
int get_config_tokens(struct token_t *t, int ignore_nl);

#endif
