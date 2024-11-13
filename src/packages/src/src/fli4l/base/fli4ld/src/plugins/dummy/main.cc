/*****************************************************************************
 *  @file src/plugins/dummy/main.cc
 *  fli4ld dummy plug-in: Main entry point.
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

#include <fli4ld/plugin/API.hh>
#include <ostream>
#include <iostream>

using namespace std;

/// Namespace for the fli4ld dummy plug-in.
namespace dummy {

/// Namespace of the V1.0 implementation of the plug-in's C++ API.
namespace V1_0 {

/**
 * Implements version 1.0 of the plug-in's C++ API.
 */
class DummyPlugin :
		virtual public fli4ld::plugin::V1_0::Plugin {
public :
	/**
	 * Creates the DummyPlugin object.
	 *
	 * @param fli4ld The fli4ld API.
	 */
	DummyPlugin(fli4ld::daemon::V1_0::Fli4ld &fli4ld) :
			m_fli4ld(fli4ld) {
		cout << "DummyPlugin: creation" << endl;
	}

	/// Destroys the DummyPlugin object.
	virtual ~DummyPlugin() override {
		cout << "DummyPlugin: destruction" << endl;
	}

	virtual std::string getPluginName() const override {
		return "dummy";
	}

	virtual fli4ld::Version getPluginVersion() const override {
		return fli4ld::Version(1, 0);
	}

private:
	/// The fli4ld API.
	fli4ld::daemon::V1_0::Fli4ld &m_fli4ld;
};

} // V_10
} // dummy

extern "C" {

/**
 * @return The DummyPlugin's maximum supported plug-in API.
 */
fli4ld::Version getProvidedPluginAPIVersion() {
	return dummy::V1_0::DummyPlugin::InterfaceVersion;
}

/**
 * @return The DummyPlugin's minimum required fli4ld API.
 */
fli4ld::Version getRequiredFli4ldAPIVersion() {
	return fli4ld::daemon::V1_0::Fli4ld::InterfaceVersion;
}

/**
 * Creates a plug-in object implementing the plug-in's C++ API.
 *
 * @param fli4ld The fli4ld API provided by fli4ld.
 * @return A plug-in object implementing the plug-in's C++ API.
 */
fli4ld::plugin::PluginMin *createPluginObject(fli4ld::daemon::Fli4ldMin &fli4ld) {
	if (fli4ld::daemon::V1_0::Fli4ld *api
			= dynamic_cast<fli4ld::daemon::V1_0::Fli4ld *>(&fli4ld)) {
		return new dummy::V1_0::DummyPlugin(*api);
	} else {
		// safety belt, should not happen due to minimum required fli4ld version
		// returned by getMinimumRequiredFli4ldAPI()
		throw fli4ld::plugin::PluginMin::Exception("fli4ld API too old");
	}
}

}
