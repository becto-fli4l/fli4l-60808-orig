/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/RegexType.hh
 *  fli4ld: Regular expression type (V1.0).
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

#ifndef FLI4LD_OP_V1_0_REGEX_TYPE_H_
#define FLI4LD_OP_V1_0_REGEX_TYPE_H_

#include <fli4ld/op/V1.0/Type.hh>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace fli4ld {
namespace op {

namespace V1_0 {

/**
 * Represents all string values that match a regular expression.
 */
class RegexType final : public Type {
public :

	/// The underlying raw type.
	using RawType = std::vector<std::string>;

	/**
	 * Creates a regular expression type.
	 *
	 * @param regex The regular expression.
	 */
	static std::shared_ptr<RegexType> create(std::string const &regex) {
		return std::shared_ptr<RegexType>{new RegexType{regex}};
	}

	virtual std::string getName() const override {
		return "RegEx[" + m_regexStr + "]";
	}

private:
	std::string const m_regexStr;
	std::regex const m_regex;

	/**
	 * Constructor.
	 *
	 * @param regex The regular expression.
	 */
	RegexType(std::string const &regex) :
			m_regexStr(regex),
			m_regex{
				regex,
				std::regex_constants::optimize} {
	}

	/**
	 * Converts the string value to a Value.
	 *
	 * @param value The string value to convert.
	 * @param result Set to a vector of strings representing the result of
	 * matching <code>value</code> against the regular expression. Each string
	 * represents a subgroup. The first string captures the entire string value.
	 * @return <code>true</code> if the conversion was successful,
	 * <code>false</code> if the string value does not match the regular
	 * expression.
	 */
	virtual bool doFromString(std::string const &value, Value &result) const
			override;
};

} // V1_0

} // op
} // fli4ld

#endif
