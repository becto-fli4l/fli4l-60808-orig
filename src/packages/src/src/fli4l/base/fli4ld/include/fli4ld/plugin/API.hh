/*****************************************************************************
 *  @file include/fli4ld/plugin/API.hh
 *  fli4ld: Plug-in API.
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

#ifndef FLI4LD_PLUGIN_API_H_
#define FLI4LD_PLUGIN_API_H_

#include <fli4ld/plugin/V1.0/Plugin.hh>

namespace fli4ld {

/// Defines the fli4ld plug-in API.
namespace plugin {

/**
 * The lowest version of the plug-in API fli4ld requires. If some plug-in being
 * loaded provides an API version less than this API's version, the plug-in is
 * unloaded again, as its interface is incompatible with this version of fli4ld.
 */
typedef V1_0::Plugin PluginMin;

/**
 * The highest version of the plug-in API fli4ld supports.
 */
typedef V1_0::Plugin PluginMax;

} // plugin
} // fli4ld

#include <fli4ld/daemon/API.hh>

extern "C" {

/**
 * Returns the version of the plug-in API the plug-in provides. If fli4ld does
 * not support this API version, the plug-in is unloaded and
 * createPluginObject() is not called. This can happen if the plug-in's API is
 * too old or too new.
 *
 * Note that in contrast to the fli4ld API, it may happen that a plug-in is
 * loaded that provides a plug-in API whose version is lower than the version
 * of the plug-in API implemented by fli4ld. This is not a problem as fli4ld
 * can work with an old plug-in by avoiding calls requiring a newer plug-in
 * API.
 *
 * @return The version of the plug-in API provided by the plug-in.
 */
fli4ld::Version getProvidedPluginAPIVersion();

/**
 * Returns the version of the fli4ld API the plug-in requires. If fli4ld
 * provides an API that is incompatible or too low, the plug-in is unloaded and
 * createPluginObject() is not called. This can happen if the plug-in is too
 * old or too new.
 *
 * Note that in contrast to the plug-in API, a plug-in is never loaded if the
 * fli4ld API version is lower than the version returned by this function. This
 * is to not require the plug-in developer to write code that works with a
 * range of different fli4ld versions.
 *
 * @return The version of the fli4ld API required by the plug-in.
 */
fli4ld::Version getRequiredFli4ldAPIVersion();

/**
 * Creates a plug-in object implementing the plug-in's C++ API. The object
 * is used in later communication from fli4ld to the plug-in. This function is
 * only called if the plug-in's and fli4ld's APIs are compatible. See the
 * documentation of getProvidedPluginAPI() and getRequiredFli4ldAPI() for
 * details.
 *
 * @param fli4ld The fli4ld API fli4ld provides. Note that fli4ld guarantees
 * that this object's API version is compatible to and greater than or equal to
 * the version returned by getRequiredFli4ldAPI().
 *
 * @return A pointer to the plug-in object implementing the fli4ld's minimum
 * plug-in interface. Note that all plug-in interfaces sharing the same
 * major number are interface-wise compatible using C++'s point of view, i.e.
 * newer interfaces extend the older ones. This is not guaranteed for
 * interfaces having different major numbers. This means that a single plug-in
 * cannot implement two incompatible plug-in APIs, i.e. two plug-in interfaces
 * with different major versions.
 *
 * @throws fli4ld::plugin::Plugin::Exception if creating plug-in object fails
 * for some reason.
 */
fli4ld::plugin::PluginMin *createPluginObject(fli4ld::daemon::Fli4ldMin &fli4ld);

}

#endif
