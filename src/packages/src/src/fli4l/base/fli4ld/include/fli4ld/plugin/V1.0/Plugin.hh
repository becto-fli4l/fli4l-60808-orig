/*****************************************************************************
 *  @file include/fli4ld/plugin/V1.0/Plugin.hh
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

#ifndef FLI4LD_PLUGIN_V1_0_PLUGIN_H_
#define FLI4LD_PLUGIN_V1_0_PLUGIN_H_

#include <fli4ld/Version.hh>
#include <fli4ld/plugin/V1.0/API.hh>
#include <string>

namespace fli4ld {
namespace plugin {

namespace V1_0 {

/**
 * Defines the C++ API of a plug-in. A fli4ld plug-in must implement this
 * interface (or a subinterface of it) in order to provide services accessible
 * by fli4ld.
 */
class Plugin {
public :

	/// The version bound to this interface.
	static constexpr Version InterfaceVersion = APIVersion;

	/**
	 * Encapsulates exceptions thrown by plug-ins.
	 */
	class Exception : public std::exception {
	public :
		/**
		 * Creates an Exception object with a message.
		 *
		 * @param message The exception message.
		 */
		Exception(std::string const &message) :
				m_message(message) {
		}

		/**
		 * @return The exception message.
		 */
		virtual const char* what() const noexcept override {
			return m_message.c_str();
		}

	protected :
		/**
		 * Creates an Exception object without a message. Subclasses are
		 * expected to override what() accordingly.
		 */
		Exception() : m_message() {
		}

	private :
		std::string m_message; ///< The exception message.
	};

	/**
	 * Destructor cleaning up behind the plug-in. Note that fli4ld guarantees
	 * that this destructor is called before the plug-in library is unloaded.
	 */
	virtual ~Plugin() {
	}

	/**
	 * Returns the plug-in's API version. The value returned must always be
	 * equal to InterfaceVersion, so do not redefine this method in an
	 * incompatible way.
	 *
	 * @return The plug-in's API version number.
	 */
	virtual Version getAPIVersion() const {
		return InterfaceVersion;
	}

	/**
	 * @return The plug-in's name.
	 */
	virtual std::string getPluginName() const = 0;

	/**
	 * Returns the plug-in's version. This version number can be arbitrary and
	 * is used by fli4ld only for status queries and the like. No version
	 * checks are ever made with regard to the value returned by this
	 * operation. However, the developer of a plug-in should increment the
	 * plug-in's version number whenever the plug-in's functionality is
	 * changed.
	 *
	 * @return The plug-in's version number.
	 */
	virtual Version getPluginVersion() const = 0;

};

} // V1_0

} // plugin
} // fli4ld

#endif
