/*----------------------------------------------------------------------------
 *  arch_comp_gzip.h - compression backend using gzip compression (RFC 1952)
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

#if !defined (MKFLI4L_ARCH_COMP_GZIP_H_)
#define MKFLI4L_ARCH_COMP_GZIP_H_

#include "arch_comp.h"

/**
 * Returns a GZIP backend.
 * @return
 *  A pointer to an arch_comp_backend_t object for gzip compression.
 */
struct arch_comp_backend_t *create_comp_gzip_backend(void);

#endif
