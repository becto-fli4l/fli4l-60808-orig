/*****************************************************************************
 * @file src/fli4ld/op/V1.0/TupleType.cc
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

#include <fli4ld/op/V1.0/TupleType.hh>
#include <fli4ld/op/V1.0/RegexType.hh>

namespace fli4ld {
namespace op {

namespace V1_0 {

std::string TupleType::getName() const {
	std::ostringstream os;
	os << "TupleOf[";
	bool first = true;
	for (std::shared_ptr<Type const> elementType : m_elementTypes) {
		if (first) {
			first = false;
		} else {
			os << ",";
		}
		os << elementType->getName();
	}
	os << "]";
	return os.str();
}

bool TupleType::doFromString(std::string const &value, Value &result) const {
	RegexType::RawType const groups{
		m_regex->fromString(value).to<RegexType::RawType>()};

	std::vector<Value> vectorResult;
	for (decltype(groups)::size_type i = 1; i < groups.size(); ++i) {
		vectorResult.push_back(m_elementTypes[i - 1]->fromString(groups[i]));
	}

	result = Value{shared_from_this(), vectorResult};
	return true;
}

} // V1_0

} // op
} // fli4ld
