/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/RangeType.hh
 *  fli4ld: Range type (V1.0).
 *
 *  Copyright (c) 2018 - fli4l-Team <team@fli4l.de>
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

#ifndef FLI4LD_OP_V1_0_RANGE_TYPE_H_
#define FLI4LD_OP_V1_0_RANGE_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents values in a given half-open range.
 * Membership is tested using operator<() on T::RawType values.
 *
 * @tparam T The underlying type (e.g. IntegerType).
 * @tparam LowerBound The first allowed value.
 * @tparam UpperBound The first disallowed value greater than LowerBound.
 */
template<
	typename T,
	typename T::RawType LowerBound,
	typename T::RawType UpperBound
>
class RangeType final : public Type {
public :

	/**
	 * @return An object of this type.
	 */
	static std::shared_ptr<RangeType<T, LowerBound, UpperBound>> create() {
		return std::shared_ptr<RangeType<T, LowerBound, UpperBound>>{
			new RangeType<T, LowerBound, UpperBound>{}};
	}

	virtual std::string getName() const override {
		std::ostringstream os;
		os << "Range[[";
		os << LowerBound;
		os << ";";
		os << UpperBound;
		os << ")]";
		return os.str();
	}

private :
	/**
	 * Default constructor.
	 */
	RangeType() = default;

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to <code>value</code> converted to the underlying type.
	 * @return <code>true</code> if the conversion was successful,
	 * <code>false</code> if the string value does not denote a value of the
	 * proper type in the allowed range.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const
			override;
};

template<
	typename T,
	typename T::RawType LowerBound,
	typename T::RawType UpperBound
>
bool RangeType<T, LowerBound, UpperBound>::doFromString(
		std::string const &value,
		Value &result) const {

	Value underlyingValue{T::get()->fromString(value)};
	typename T::RawType const rawValue =
		underlyingValue.to<typename T::RawType>();
	if (rawValue < LowerBound || !(rawValue < UpperBound)) {
		return false;
	} else {
		result = Value{shared_from_this(), rawValue};
		return true;
	}
}

} // V1_0

} // op
} // fli4ld

#endif
