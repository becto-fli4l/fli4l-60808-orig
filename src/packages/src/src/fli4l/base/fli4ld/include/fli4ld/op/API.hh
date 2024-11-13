/*****************************************************************************
 *  @file include/fli4ld/op/API.hh
 *  fli4ld: Operation API.
 *
 *  Copyright (c) 2018 - fli4l-Team <team@fli4l.de>
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
 *****************************************************************************
 */

#ifndef FLI4LD_OP_API_H_
#define FLI4LD_OP_API_H_

#include <fli4ld/op/V1.0/API.hh>

/// Namespace for fli4ld, the fli4l daemon.
namespace fli4ld {

/// Defines the fli4ld operation API.
namespace op {

/**
 * The minimum operation API fli4ld provides.
 */
typedef V1_0::APIVersion APIVersionMin;

} // op
} // fli4ld

#endif
