/*****************************************************************************
 * @file src/fli4ld/op/V1.0/ListType.cc
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

#include <fli4ld/op/V1.0/ListType.hh>

namespace fli4ld {
namespace op {

namespace V1_0 {

bool ListType::doFromString(std::string const &value, Value &result) const {
	std::list<Value> resultList;
	if (!value.empty()) {
		std::sregex_token_iterator
				first{value.begin(), value.end(), m_separator, -1},
				last{};

		while (first != last) {
			resultList.push_back(m_elementType->fromString(*first));
			++first;
		}
	}
	result = Value{shared_from_this(), resultList};
	return true;
}

} // V1_0

} // op
} // fli4ld
