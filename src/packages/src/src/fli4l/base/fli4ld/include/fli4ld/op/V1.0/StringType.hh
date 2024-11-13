/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/StringType.hh
 *  fli4ld: String type (V1.0).
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

#ifndef FLI4LD_OP_V1_0_STRING_TYPE_H_
#define FLI4LD_OP_V1_0_STRING_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <memory>
#include <string>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents all (string) values.
 */
class StringType final : public Type {
public :

	/// The underlying raw type.
	using RawType = std::string;

	/**
	 * Returns a shared pointer to the singleton instance of this type.
	 */
	static std::shared_ptr<StringType> get() {
		return std::shared_ptr<StringType>{&sm_instance, [](StringType *) {}};
	}

	virtual std::string getName() const override;

private :
	static StringType sm_instance;

	/**
	 * Default constructor.
	 */
	StringType() = default;

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to <code>value</code>.
	 * @return Always <code>true</code>.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const
			override {
		result = Value{get(), value};
		return true;
	}
};

} // V1_0

} // op
} // fli4ld

#endif
