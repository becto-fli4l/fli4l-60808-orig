/*****************************************************************************
 *  @file include/fli4ld/daemon/V1.0/Daemon.hh
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

#ifndef FLI4LD_DAEMON_V1_0_DAEMON_H_
#define FLI4LD_DAEMON_V1_0_DAEMON_H_

#include <fli4ld/Version.hh>
#include <fli4ld/daemon/V1.0/API.hh>
#include <fli4ld/cmd/V1.0/Command.hh>
#include <memory>

namespace fli4ld {
namespace daemon {

/// Defines the fli4ld daemon API version 1.0.
namespace V1_0 {

/**
 * Defines the C++ daemon API of fli4ld. It is implemented by fli4ld and passed
 * to the plug-in during initialization.
 */
class Fli4ld {
public :

	/// The version bound to this interface.
	static constexpr Version InterfaceVersion = APIVersion;

	/// Destructor.
	virtual ~Fli4ld() {
	}

	/**
	 * Returns the fli4ld's daemon API version. The value returned must always
	 * be equal to InterfaceVersion, so do not redefine this method in an
	 * incompatible way.
	 *
	 * @return The fli4ld's daemon API version number.
	 */
	virtual Version getAPIVersion() const {
		return InterfaceVersion;
	}

	/**
	 * Queues a command for later execution. The execution of queued commands
	 * is strictly serialized, so accessing shared data need not be
	 * synchronized.
	 *
	 * @param cmd The command to queue.
	 * @param isCleanup If true, the command is queued even if fli4ld is in the
	 * middle of being stopped.
	 *
	 * @return True if queuing the command has been successful, else false.
	 * Queuing a command fails if fli4ld is being terminated and "isCleanup" is
	 * not true.
	 *
	 * @throws invalid_argument if an empty pointer is passed.
	 */
	virtual bool invokeLater(
			std::shared_ptr<cmd::V1_0::Command> cmd,
			bool isCleanup = false
	)
		= 0;

};

} // V1_0

} // daemon
} // fli4ld

#endif
