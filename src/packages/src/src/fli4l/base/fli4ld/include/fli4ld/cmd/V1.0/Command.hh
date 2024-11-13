/*****************************************************************************
 *  @file include/fli4ld/cmd/V1.0/Command.hh
 *  fli4ld: Command interface (V1.0).
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

#ifndef FLI4LD_CMD_V1_0_COMMAND_H_
#define FLI4LD_CMD_V1_0_COMMAND_H_

#include <memory>

namespace fli4ld {
namespace cmd {

namespace V1_0 {

/**
 * Represents a generic command.
 *
 * A command is always managed by a shared pointer. Because of this, this class
 * inherits from enable_shared_from_this to make shared_from_this() work.
 */
class Command :
		public std::enable_shared_from_this<Command> {
public :

	/// Destructor.
	virtual ~Command() {
	}

	/// Executes the command.
	virtual void run() = 0;

};

} // V1_0

} // cmd
} // fli4ld

#endif
