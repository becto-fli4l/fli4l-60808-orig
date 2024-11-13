/*****************************************************************************
 *  @file src/fli4ld/daemon/V1.0/DaemonImpl.cc
 *  fli4ld: Daemon API (V1.0).
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

#include "DaemonImpl.hh"

namespace fli4ld {
namespace daemon {

namespace V1_0 {

bool Fli4ldImpl::invokeLater(
		std::shared_ptr<cmd::V1_0::Command> cmd,
		bool isCleanup
)
{
	return m_processor.queue(cmd, isCleanup);
}

} // V1_0

} // daemon
} // fli4ld
