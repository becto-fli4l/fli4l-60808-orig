/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/Type.hh
 *  fli4ld: Type interface (V1.0).
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

#ifndef FLI4LD_OP_V1_0_TYPE_H_
#define FLI4LD_OP_V1_0_TYPE_H_

#include <fli4ld/op/V1.0/Value.hh>
#include <memory>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents the type of a parameter or value.
 */
class Type : public std::enable_shared_from_this<Type> {
public :

	/// Destructor.
	virtual ~Type() = default;

	/**
	 * @return The name of this type.
	 */
	virtual std::string getName() const = 0;

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @return The Value object.
	 *
	 * @throws std::domain_error if the value does not conform to this type.
	 * The exception object also contains error messages of nested conversions
	 * (if applicable).
	 */
	Value fromString(std::string const &value) const;

private :

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Retrieves the conversion result.
	 * @return <code>true</code> iff the conversion was successful.
	 *
	 * @throws std::domain_error if some nested conversion fails with an
	 * exception.
	 */
	virtual bool doFromString(std::string const &value, Value &result)
			const = 0;
};

} // V1_0

} // op
} // fli4ld

#endif
