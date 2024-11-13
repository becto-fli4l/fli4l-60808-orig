/*****************************************************************************
 * @file src/fli4ld/op/V1.0/IntegerType.cc
 *  fli4ld: Integer type (V1.0).
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

#include <fli4ld/op/V1.0/IntegerType.hh>
#include <sstream>
#include <stdexcept>

namespace fli4ld {
namespace op {

namespace V1_0 {

IntegerType IntegerType::sm_instance;

std::string IntegerType::getName() const {
	return "Integer";
}

bool IntegerType::doFromString(std::string const &value, Value &result) const {
	std::istringstream is(value);
	int i;
	is >> i;
	if (is.fail() || !is.eof()) {
		return false;
	} else {
		result = Value{get(), i};
		return true;
	}
}

} // V1_0

} // op
} // fli4ld
