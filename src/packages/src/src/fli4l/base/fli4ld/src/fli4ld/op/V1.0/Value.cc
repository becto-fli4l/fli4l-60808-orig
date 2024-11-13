/*****************************************************************************
 * @file src/fli4ld/op/V1.0/Value.cc
 *  fli4ld: Value (V1.0).
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

#include <fli4ld/op/V1.0/Value.hh>

namespace fli4ld {
namespace op {

namespace V1_0 {

namespace {

/**
 * Adapts a ValueVisitor to std::visit().
 */
struct VisitorAdaptor {
	ValueVisitor &m_v;

	VisitorAdaptor(ValueVisitor &v) : m_v(v) {
	}

	void operator()(std::string const &value) {
		m_v.handleString(value);
	}

	void operator()(int value) {
		m_v.handleInteger(value);
	}

	void operator()(bool value) {
		m_v.handleBoolean(value);
	}

	void operator()(std::vector<std::string> const &value) {
		m_v.handleMatchedGroups(value);
	}

	void operator()(std::list<Value> const &value) {
		m_v.handleList(value);
	}

	void operator()(std::vector<Value> const &value) {
		m_v.handleTuple(value);
	}
};

}

void Value::accept(ValueVisitor &v) const {
	std::visit(VisitorAdaptor(v), m_value);
}

} // V1_0

} // op
} // fli4ld
