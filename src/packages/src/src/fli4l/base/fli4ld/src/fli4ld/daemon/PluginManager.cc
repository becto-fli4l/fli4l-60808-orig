/*****************************************************************************
 *  @file src/fli4ld/daemon/PluginManager.cc
 *  fli4ld: Manages plug-ins.
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

#include "PluginManager.hh"
#include <fli4ld/VersionRange.hh>
#include <sstream>
#include <dlfcn.h>

using namespace std;

namespace fli4ld {
namespace daemon {

PluginManager::LoadingPluginFailed::LoadingPluginFailed(
	string const &libName,
	string const &reason
) :
	m_message("Loading plug-in library " + libName + " failed: " + reason)
{
}

/**
 * Encapsulates a library handle returned by dlopen().
 */
class PluginManager::LibHandle final {
public :

	/**
	 * Creates a LibHandle object.
	 *
	 * @param libHandle The library handle returned by dlopen(). It will be
	 * closed by dlclose() in the destructor if non-NULL.
	 */
	explicit LibHandle(void *libHandle) :
			m_libHandle(libHandle) {
	}

	/**
	 * Destructor closing a non-NULL library handle using dlclose().
	 */
	~LibHandle() {
		if (isValid()) {
			dlclose(m_libHandle);
		}
	}

	/**
	 * @return True if the library handle is valid (i.e. non-NULL), else false.
	 */
	bool isValid() const {
		return m_libHandle != NULL;
	}

	/**
	 * @return The underlying library handle.
	 */
	void *getHandle() const {
		return m_libHandle;
	}

private :
	void *m_libHandle; ///< The underlying library handle.
};

/**
 * Wraps a loaded plug-in API object.
 */
class PluginManager::PluginWrapper final {
public :
	/**
	 * Creates a PluginWrapper.
	 *
	 * @param libHandle The handle of the shared object.
	 * @param module The plug-in API object.
	 */
	PluginWrapper(
			shared_ptr<LibHandle> libHandle,
			shared_ptr<plugin::PluginMin> module
	) :
			m_libHandle(libHandle),
			m_module(module) {
	}

	/**
	 * @return The plug-in API object.
	 */
	shared_ptr<plugin::PluginMin> getPlugin() {
		return m_module;
	}

	/**
	 * @return The plug-in API object (const version).
	 */
	shared_ptr<plugin::PluginMin const> getPlugin() const {
		return m_module;
	}

private :
	/// The library handle returned by dlopen().
	shared_ptr<LibHandle> m_libHandle;
	/// The plug-in API object.
	shared_ptr<plugin::PluginMin> m_module;
};

/**
 * Type of pointer to function returning a plug-in's provided plug-in API
 * version.
 */
struct PluginManager::FuncGetProvidedPluginAPIVersion {
	/// The function's name.
	static constexpr char const sm_name[] = "getProvidedPluginAPIVersion";
	/// The function's type.
	typedef Version Type();
};

constexpr char const PluginManager::FuncGetProvidedPluginAPIVersion::sm_name[];

/**
 * Type of pointer to function returning a plug-in's required fli4ld API
 * version.
 */
struct PluginManager::FuncGetRequiredFli4ldAPIVersion {
	/// The function's name.
	constexpr static char const sm_name[] = "getRequiredFli4ldAPIVersion";
	/// The function's type.
	typedef Version Type();
};

constexpr char const PluginManager::FuncGetRequiredFli4ldAPIVersion::sm_name[];

/**
 * Describes a function returning a plug-in's API object.
 */
struct PluginManager::FuncCreatePluginObject {
	/// The function's name.
	static constexpr char const sm_name[] = "createPluginObject";
	/// The function's type.
	typedef plugin::PluginMin *Type(daemon::Fli4ldMin &fli4ld);
};

constexpr char const PluginManager::FuncCreatePluginObject::sm_name[];

template<typename F>
typename F::Type *PluginManager::getExportedFunction(
		std::string const &libName,
		std::shared_ptr<LibHandle> libHandle
)
{
	typename F::Type *pointer;

	// This horrible type cast is necessary because ISO C++ does not allow
	// casts between object and function pointers, but current POSIX API
	// requires them... :-(
	*reinterpret_cast<void **>(&pointer)
			= dlsym(libHandle->getHandle(), F::sm_name);

	if (pointer) {
		return pointer;
	} else {
		ostringstream os;
		os << "Incompatible library interface: Function "
				<< F::sm_name << " not found";
		throw LoadingPluginFailed(libName, os.str());
	}
}

PluginManager::PluginManager(Fli4ldMin &fli4ld) :
	m_fli4ld(fli4ld)
{
}

shared_ptr<plugin::PluginMin> PluginManager::load(string const &libName)
{
	shared_ptr<LibHandle> libHandle(new LibHandle(
		dlopen(libName.c_str(), RTLD_LAZY | RTLD_LOCAL)
	));
	if (!libHandle->isValid()) {
		throw LoadingPluginFailed(libName, "Library could not be opened");
	}

	auto funcGetProvidedPluginAPIVersion
			= getExportedFunction<FuncGetProvidedPluginAPIVersion>(libName, libHandle);
	auto funcGetRequiredFli4ldAPIVersion
			= getExportedFunction<FuncGetRequiredFli4ldAPIVersion>(libName, libHandle);
	auto funcCreatePluginObject
			= getExportedFunction<FuncCreatePluginObject>(libName, libHandle);

	Version const providedPluginAPI = (*funcGetProvidedPluginAPIVersion)();
	if (!providedPluginAPI.isAPICompatibleTo(plugin::PluginMin::InterfaceVersion)
			|| providedPluginAPI < plugin::PluginMin::InterfaceVersion) {
		ostringstream os;
		os << "Incompatible plug-in API: plug-in provides version "
				<< providedPluginAPI << ", fli4ld supports the version range "
				<< VersionRange(plugin::PluginMin::InterfaceVersion,
						plugin::PluginMax::InterfaceVersion);
		throw LoadingPluginFailed(libName, os.str());
	}

	Version const requiredFli4ldAPI = (*funcGetRequiredFli4ldAPIVersion)();
	if (!requiredFli4ldAPI.isAPICompatibleTo(m_fli4ld.getAPIVersion())
			|| m_fli4ld.getAPIVersion() < requiredFli4ldAPI) {
		ostringstream os;
		os << "Incompatible fli4ld API: plug-in requires at least version "
				<< requiredFli4ldAPI << ", fli4ld provides version "
				<< m_fli4ld.getAPIVersion();
		throw LoadingPluginFailed(libName, os.str());
	}

	try {
		if (shared_ptr<plugin::PluginMin> module = shared_ptr<plugin::PluginMin>(
					(*funcCreatePluginObject)(m_fli4ld)
				)) {
			// plug-in API object successfully created
			shared_ptr<PluginWrapper> wrapper(new PluginWrapper(
					libHandle,
					module
			));
			m_plugins.push_back(wrapper);
			return wrapper->getPlugin();
		} else {
			// empty pointer returned, should never happen
			throw LoadingPluginFailed(
				libName,
				"Creation of plug-in object failed for unknown reasons"
			);
		}
	} catch (plugin::PluginMin::Exception const &e) {
		ostringstream os;
		os << "Creation of plug-in object failed: " << e.what();
		throw LoadingPluginFailed(libName, os.str());
	}
}

} // daemon
} // fli4ld
