/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/TupleType.hh
 *  fli4ld: Tuple type (V1.0).
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

#ifndef FLI4LD_OP_V1_0_TUPLE_TYPE_H_
#define FLI4LD_OP_V1_0_TUPLE_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <fli4ld/op/V1.0/RegexType.hh>
#include <initializer_list>
#include <memory>
#include <regex>
#include <vector>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents tuples of values.
 */
class TupleType final : public Type {
public :

	/// The underlying raw type.
	using RawType = std::vector<Value>;

	/**
	 * Creates a tuple type.
	 *
	 * @param elementTypes The underlying element types.
	 * @param regex A regular expression extracting the tuple elements.
	 */
	static std::shared_ptr<TupleType> create(
			std::initializer_list<std::shared_ptr<Type const>> elementTypes,
			std::string const &regex) {
		return std::shared_ptr<TupleType>{
			new TupleType{elementTypes, regex}};
	}

	virtual std::string getName() const override;

private:
	std::vector<std::shared_ptr<Type const>> m_elementTypes;
	std::shared_ptr<RegexType> m_regex;

	/**
	 * Constructor.
	 *
	 * @param elementTypes The underlying element types.
	 * @param regex A regular expression extracting the tuple elements.
	 */
	TupleType(
			std::initializer_list<std::shared_ptr<Type const>> elementTypes,
			std::string const &regex) :
			m_elementTypes{elementTypes.begin(), elementTypes.end()},
			m_regex{RegexType::create(regex)} {
	}

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to a vector of Values represented by
	 * <code>value</code>.
	 * @return Always <code>true</code>.
	 *
	 * @throws std::domain_error if converting some tuple element fails with an
	 * exception.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const
			override;
};

} // V1_0

} // op
} // fli4ld

#endif
