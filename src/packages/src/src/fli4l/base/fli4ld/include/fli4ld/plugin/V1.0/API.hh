/*****************************************************************************
 *  @file include/fli4ld/plugin/V1.0/API.hh
 *  fli4ld: Plug-in API (V1.0).
 *
 *  Copyright (c) 2015 - fli4l-Team <team@fli4l.de>
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

#ifndef FLI4LD_PLUGIN_V1_0_API_H_
#define FLI4LD_PLUGIN_V1_0_API_H_

#include <fli4ld/Version.hh>

namespace fli4ld {
namespace plugin {

/// Defines the fli4ld plug-in API version 1.0.
namespace V1_0 {

/// The version bound to this API.
constexpr Version APIVersion = Version(1, 0);

} // V1_0

} // plugin
} // fli4ld

#include <fli4ld/plugin/V1.0/Plugin.hh>

#endif
