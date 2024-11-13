/*----------------------------------------------------------------------------
 *  boottype_integrated.h - functions for handling integrated boot types
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

#ifndef MKFLI4L_BOOTTYPE_INTEGRATED_H_
#define MKFLI4L_BOOTTYPE_INTEGRATED_H_

#include "boottype.h"

/**
 * Returns a backend for integrated boot types.
 * @return
 *  A pointer to an boot_type_backend_t object for integrated boot types.
 */
struct boot_type_backend_t *create_integrated_boot_type_backend(void);

#endif
