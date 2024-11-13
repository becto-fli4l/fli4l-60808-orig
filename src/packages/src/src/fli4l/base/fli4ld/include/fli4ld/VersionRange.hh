/*****************************************************************************
 *  @file include/fli4ld/VersionRange.hh
 *  fli4ld: Defines version number ranges.
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

#ifndef FLI4LD_VERSION_RANGE_H_
#define FLI4LD_VERSION_RANGE_H_

#include <fli4ld/Version.hh>
#include <utility>
#include <ostream>
#include <sstream>

/// Namespace for fli4ld, the fli4l daemon.
namespace fli4ld {

/**
 * Represents a version range. Version ranges are closed intervals delimited
 * by version numbers.
 *
 * Note that objects of this class are immutable.
 */
class VersionRange final {
public :

	/**
	 * Creates a VersionRange object.
	 *
	 * @param low The low version (inclusive).
	 * @param high The high version (inclusive).
	 */
	//@{
	constexpr VersionRange(Version const &low, Version const &high) :
			m_low(low),
			m_high(high) {
	}
	VersionRange(Version &&low, Version &&high) :
			m_low(std::move(low)),
			m_high(std::move(high)) {
	}
	//@}

	/**
	 * @return The low version.
	 */
	constexpr Version const &getLow() const {
		return m_low;
	}

	/**
	 * @return The high version.
	 */
	constexpr Version const &getHigh() const {
		return m_high;
	}

	/**
	 * Returns true if this VersionRange object contains the passed Version.
	 *
	 * @param version The Version object to check.
	 * @return True if the Version object is part of this VersionRange, else
	 * false.
	 */
	bool contains(Version const &version) const {
		return version >= m_low && version <= m_high;
	}

	/**
	 * Returns true if this VersionRange object and the passed one represent
	 * the same version range, i.e. the low and high versions are pairwise the
	 * same.
	 *
	 * Note that this method constitutes an equivalence relation.
	 *
	 * @param other The other VersionRange object to compare with.
	 * @return True if both VersionRange objects represent the same version
	 * range.
	 */
	bool operator==(VersionRange const &other) const {
		return m_low == other.m_low && m_high == m_high;
	}

	/**
	 * Returns true if this VersionRange object and the passed one represent
	 * different version ranges, i.e. if the low or the high versions (or both)
	 * are pairwise unequal.
	 *
	 * @param other The other VersionRange object to compare with.
	 * @return True if both VersionRange objects represent different version
	 * ranges.
	 */
	bool operator!=(VersionRange const &other) const {
		return !(*this == other);
	}

	/**
	 * @return A textual description of this VersionRange object.
	 */
	std::string toString() const {
		std::ostringstream os;
		os << *this;
		return os.str();
	}

	/**
	 * Writes a textual description of a VersionRange object to an output
	 * stream.
	 *
	 * @param os The output stream.
	 * @param versionRange The VersionRange object.
	 * @return os
	 */
	friend std::ostream &operator<<(
			std::ostream &os,
			VersionRange const &versionRange
	) {
		return os << '[' << versionRange.getLow() << ',' << versionRange.getHigh() << ']';
	}

private :
	Version const m_low; ///< The low version.
	Version const m_high; ///< The high version.
};

}

#endif
