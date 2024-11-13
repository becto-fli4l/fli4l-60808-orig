/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/ChoiceType.hh
 *  fli4ld: Choice type (V1.0).
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

#ifndef FLI4LD_OP_V1_0_CHOICE_TYPE_H_
#define FLI4LD_OP_V1_0_CHOICE_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <initializer_list>
#include <memory>
#include <regex>
#include <vector>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents polymorphic values.
 */
class ChoiceType final : public Type {
public :

	/**
	 * Creates a choice type.
	 *
	 * @param alternativeTypes The underlying alternative types.
	 */
	static std::shared_ptr<ChoiceType> create(
			std::initializer_list<std::shared_ptr<Type>> alternativeTypes) {
		return std::shared_ptr<ChoiceType>{
			new ChoiceType{alternativeTypes}};
	}

	virtual std::string getName() const override;

private:
	std::list<std::shared_ptr<Type>> m_alternativeTypes;

	/**
	 * Constructor.
	 *
	 * @param alternativeTypes The underlying alternative types.
	 */
	ChoiceType(
			std::initializer_list<std::shared_ptr<Type>> alternativeTypes) :
			m_alternativeTypes{
				alternativeTypes.begin(), alternativeTypes.end()} {
	}

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to <code>value</code> converted to one of the possible
	 * types.
	 * @return <code>true</code> if the conversion was successful,
	 * <code>false</code> if this is the empty choice.
	 *
	 * @throws std::domain_error iff the string value does not conform to
	 * any of the possible types.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const override;
};

} // V1_0

} // op
} // fli4ld

#endif
