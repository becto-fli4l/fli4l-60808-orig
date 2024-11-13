/*----------------------------------------------------------------------------
 *  arch_type_tar.h - functions for creating TAR archives
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

#if !defined (MKFLI4L_ARCH_TYPE_TAR_H_)
#define MKFLI4L_ARCH_TYPE_TAR_H_

#include "arch_type.h"

/**
 * Returns a TAR backend.
 * @return
 *  A pointer to an arch_type_backend_t object controlling a TAR archive.
 */
struct arch_type_backend_t *create_tar_backend(void);

#endif
