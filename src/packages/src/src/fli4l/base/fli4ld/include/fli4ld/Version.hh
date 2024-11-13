/*****************************************************************************
 *  @file include/fli4ld/Version.hh
 *  fli4ld: Defines version numbers.
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

#ifndef FLI4LD_VERSION_H_
#define FLI4LD_VERSION_H_

#include <ostream>
#include <sstream>

/// Namespace for fli4ld, the fli4l daemon.
namespace fli4ld {

/**
 * Represents a version number consisting of a major and a minor version. The
 * idea is that major version changes represent incompatible API changes (e.g.
 * deletion of functions or changing a function's signature) and minor version
 * changes represent compatible API changes (e.g. addition of functions or
 * semantic-preserving implementation changes). This class also provides a
 * total order on version numbers as well as a "is-compatible-to" equivalence.
 *
 * Note that objects of this class are immutable.
 */
class Version final {
public :

	/**
	 * Creates a Version object.
	 *
	 * @param major The major version.
	 * @param minor The minor version.
	 */
	constexpr Version(unsigned const major, unsigned const minor) :
			m_major(major),
			m_minor(minor) {
	}

	/**
	 * @return The major version.
	 */
	constexpr unsigned getMajor() const {
		return m_major;
	}

	/**
	 * @return The minor version.
	 */
	constexpr unsigned getMinor() const {
		return m_minor;
	}

	/**
	 * Returns true if this Version object and the passed one are
	 * API-compatible. This is the case if the major versions are the same.
	 * 
	 * Note that this method constitutes an equivalence relation.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if both Version objects are API-compatible, else false.
	 */
	bool isAPICompatibleTo(Version const &other) const {
		return getMajor() == other.getMajor();
	}

	/**
	 * Returns true if this Version object and the passed one represent the
	 * same version, i.e. the major and minor versions are pairwise the same.
	 * 
	 * Note that this method constitutes an equivalence relation.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if both Version objects represent the same version.
	 */
	bool operator==(Version const &other) const {
		return m_major == other.m_major && m_minor == other.m_minor;
	}

	/**
	 * Returns true if this Version object and the passed one represent
	 * different versions, i.e. if the major or the minor versions (or both)
	 * are pairwise unequal.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if both Version objects represent different versions.
	 */
	bool operator!=(Version const &other) const {
		return !(*this == other);
	}

	/**
	 * Returns true if this Version object represents an earlier version than
	 * the passed one, i.e. if either its major version is numerically less
	 * than the other's one, or if both major versions are the same and its
	 * minor version is numerically less than the other's one. In other words,
	 * this Version object is earlier than the one passed if it is smaller with
	 * respect to the product order on tuples (major, minor).
	 * 
	 * Note that this method constitutes a strict total order.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if this Version object represents an earlier version than
	 * the passed one.
	 */
	bool operator<(Version const &other) const {
		return m_major < other.m_major ||
			(m_major == other.m_major && m_minor < other.m_minor);
	}

	/**
	 * Returns true if this Version object is either equal to or earlier than
	 * the passed Version object.
	 * 
	 * Note that this method constitutes a non-strict total order.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if this Version object is either equal to or earlier than
	 * the passed Version object.
	 */
	bool operator<=(Version const &other) const {
		return !(other < *this);
	}

	/**
	 * Returns true if this Version object represents a later version than the
	 * passed one, i.e. if either its major version is numerically greater than
	 * the other's one, or if both major versions are the same and its
	 * minor version is numerically greater than the other's one. In other
	 * words, this Version object is later than the one passed if it is greater
	 * with respect to the product order on tuples (major, minor).
	 * 
	 * Note that this method constitutes a strict total order.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if this Version object represents a later version than
	 * the passed one.
	 */
	bool operator>(Version const &other) const {
		return other < *this;
	}

	/**
	 * Returns true if this Version object is either equal to or later than the
	 * passed Version object.
	 * 
	 * Note that this method constitutes a non-strict total order.
	 *
	 * @param other The other Version object to compare with.
	 * @return True if this Version object is either equal to or later than the
	 * passed Version object.
	 */
	bool operator>=(Version const &other) const {
		return !(*this < other);
	}

	/**
	 * @return A textual description of this Version object.
	 */
	std::string toString() const {
		std::ostringstream os;
		os << *this;
		return os.str();
	}

	/**
	 * Writes a textual description of a Version object to an output stream.
	 *
	 * @param os The output stream.
	 * @param version The version object.
	 * @return os
	 */
	friend std::ostream &operator<<(std::ostream &os, Version const &version) {
		return os << version.getMajor() << '.' << version.getMinor();
	}

private :
	unsigned const m_major; ///< The major version.
	unsigned const m_minor; ///< The minor version.
};

}

#endif
