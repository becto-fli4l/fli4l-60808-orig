/*----------------------------------------------------------------------------
 *  mk_syslinux.h - functions for creating syslinux.cfg file
 *
 *  Copyright (c) 2003 - Gernot Miksch
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
 *  Creation:    2003-02-27  gm
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#ifndef MKFLI4L_MK_SYSLINUX_H_
#define MKFLI4L_MK_SYSLINUX_H_

#define ERR_MKSYSL_TMPL         -2
#define ERR_MKSYSL_CFG          -3
#define ERR_MKSYSL_FD0          -4
#define ERR_MKSYSL_APPEND       -5

int mk_syslinux (char * fname_s,char * fname_d);

#endif
