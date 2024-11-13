/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/API.hh
 *  fli4ld: Operation API (V1.0).
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

#ifndef FLI4LD_OP_V1_0_API_H_
#define FLI4LD_OP_V1_0_API_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <fli4ld/op/V1.0/StringType.hh>
#include <fli4ld/op/V1.0/RegexType.hh>
#include <fli4ld/op/V1.0/IntegerRangeType.hh>
#include <fli4ld/op/V1.0/IntegerType.hh>
#include <fli4ld/op/V1.0/ListType.hh>
#include <fli4ld/Version.hh>

namespace fli4ld {
namespace op {

/// Defines the fli4ld operation API version 1.0.
namespace V1_0 {

/// The version bound to this API.
constexpr Version APIVersion = Version(1, 0);

} // V1_0

} // op
} // fli4ld

#endif
