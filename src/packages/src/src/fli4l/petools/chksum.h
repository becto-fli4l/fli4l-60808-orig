/**************************************************************************** 
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l petools.
 *
 *  petools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  petools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with petools.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#if !defined (FLI4L_PETOOLS_CHKSUM_H)
#define FLI4L_PETOOLS_CHKSUM_H

#include "common.h"

int petools_update_checksum(struct petools_file_t *h);

#endif
