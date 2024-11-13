/*----------------------------------------------------------------------------
 *  buildinfo.h - functions for generating build information
 *
 *  Copyright (c) 2012-2016 - fli4l-Team <team@fli4l.de>
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

#ifndef MKFLI4L_BUILDINFO_H_
#define MKFLI4L_BUILDINFO_H_

/**
 * Sets ARCH to the architecture of the files as found in arch.txt.
 */
extern int set_arch(void);

/**
 * Adds information about the current build into rc.cfg.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int create_build_info(void);

#endif
