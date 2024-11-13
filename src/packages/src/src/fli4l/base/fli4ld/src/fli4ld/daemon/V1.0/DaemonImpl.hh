/*****************************************************************************
 *  @file src/fli4ld/daemon/V1.0/DaemonImpl.hh
 *  fli4ld: Implements the C++ API of fli4ld usable by plug-ins.
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

#include <fli4ld/daemon/V1.0/Daemon.hh>
#include <fli4ld/cmd/V1.0/API.hh>

namespace fli4ld {
namespace daemon {

namespace V1_0 {

/**
 * Implements the C++ API of fli4ld usable by plug-ins.
 */
class Fli4ldImpl : public V1_0::Fli4ld {
public :

	/**
	 * Starts the associated command processor.
	 *
	 * @return True on success, else false.
	 */
	bool startProcessor() {
		return m_processor.start();
	}

	/**
	 * Stops the associated command processor.
	 */
	void stopProcessor() {
		m_processor.stop();
	}

	virtual bool invokeLater(
			std::shared_ptr<cmd::V1_0::Command> cmd,
			bool isCleanup = false
	) override;

private :
	/// The command processor to use.
	cmd::V1_0::CommandProcessor m_processor;

};

}

} // daemon
} // fli4ld
