/*----------------------------------------------------------------------------
 *  unixtypes.h - definitions for some Unix types under Windows
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
 *----------------------------------------------------------------------------
 */

#if !defined (LIBMKFLI4L_WIN32_UNIXTYPES_H_)
#define LIBMKFLI4L_WIN32_UNIXTYPES_H_

#if defined(PLATFORM_mingw32)

#include <stdint.h>

typedef uint32_t uid_t;
typedef uint32_t gid_t;

#endif

#endif
