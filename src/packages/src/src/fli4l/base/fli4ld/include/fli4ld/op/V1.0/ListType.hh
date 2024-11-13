/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/ListType.hh
 *  fli4ld: List type (V1.0).
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

#ifndef FLI4LD_OP_V1_0_LIST_TYPE_H_
#define FLI4LD_OP_V1_0_LIST_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <list>
#include <memory>
#include <regex>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents lists of values.
 */
class ListType final : public Type {
public :

	/// The underlying raw type.
	using RawType = std::list<Value>;

	/**
	 * Creates a list type.
	 *
	 * @param elementType The underlying element type.
	 * @param separator A regular expression separating the list elements.
	 */
	static std::shared_ptr<ListType> create(
			std::shared_ptr<Type const> elementType,
			std::string const &separator) {
		return std::shared_ptr<ListType>{new ListType{elementType, separator}};
	}

	virtual std::string getName() const override {
		return "ListOf[" + m_elementType->getName() + "]";
	}

private:
	std::shared_ptr<Type const> m_elementType;
	std::regex const m_separator;

	/**
	 * Constructor.
	 *
	 * @param elementType The underlying element type.
	 * @param separator A regular expression separating the list elements.
	 */
	ListType(
			std::shared_ptr<Type const> elementType,
			std::string const &separator) :
			m_elementType{elementType},
			m_separator{
				separator,
				std::regex_constants::nosubs | std::regex_constants::optimize} {
	}

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to a list of Values represented by <code>value</code>.
	 * @return Always <code>true</code>.
	 *
	 * @throws std::domain_error if converting some list element fails with an
	 * exception.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const
			override;
};

} // V1_0

} // op
} // fli4ld

#endif
