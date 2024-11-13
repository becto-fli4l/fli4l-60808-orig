/*****************************************************************************
 * @file src/fli4ld/op/V1.0/ChoiceType.cc
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

#include <fli4ld/op/V1.0/ChoiceType.hh>

namespace fli4ld {
namespace op {

namespace V1_0 {

std::string ChoiceType::getName() const {
	std::ostringstream os;
	os << "ChoiceOf[";
	bool first = true;
	for (std::shared_ptr<Type const> alternativeType : m_alternativeTypes) {
		if (first) {
			first = false;
		} else {
			os << "|";
		}
		os << alternativeType->getName();
	}
	os << "]";
	return os.str();
}

bool ChoiceType::doFromString(std::string const &value, Value &result) const {
	std::string errorMessage;

	bool first = true;
	for (std::shared_ptr<Type const> type : m_alternativeTypes) {
		try {
			result = type->fromString(value);
			return true;
		} catch (std::domain_error const &ex) {
			if (first) {
				first = false;
			} else {
				errorMessage += "\nand ";
			}
			errorMessage += ex.what();
		}
	}

	if (errorMessage.empty()) {
		return false;
	} else {
		throw std::domain_error(errorMessage);
	}
}

} // V1_0

} // op
} // fli4ld
