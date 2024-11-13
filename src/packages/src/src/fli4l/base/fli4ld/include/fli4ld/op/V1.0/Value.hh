/*****************************************************************************
 *  @file include/fli4ld/op/V1.0/Value.hh
 *  fli4ld: Value interface (V1.0).
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

#ifndef FLI4LD_OP_V1_0_VALUE_H_
#define FLI4LD_OP_V1_0_VALUE_H_

#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace fli4ld {
namespace op {

namespace V1_0 {

class Type;
class ValueVisitor;

/**
 * Represents some typed value.
 */
class Value final {
public :

	/// Destructor.
	~Value() = default;

	/// Default constructor.
	Value() = default;

	/// Copy constructor.
	Value(Value const &) = default;

	/// Move constructor.
	Value(Value &&) = default;

	/// Copy assignment operator.
	Value &operator=(Value const &) = default;

	/// Move assignment operator.
	Value &operator=(Value &&) = default;

	/**
	 * Forwarding constructor.
	 * Creates the RawValue member with passed parameters.
	 *
	 * @param type The underlying type.
	 * @param values The parameters passed to the RawValue constructor.
	 */
	template <typename... T> Value(
			std::shared_ptr<Type const> type,
			T &&... values) :
			m_type{type},
			m_value{std::forward<T>(values)...} {
	}

	/**
	 * @return The underlying type.
	 */
	std::shared_ptr<Type const> getType() const {
		return m_type;
	}

	/**
	 * Accepts a ValueVisitor.
	 *
	 * @param v The visitor to accept.
	 */
	void accept(ValueVisitor &v) const;

	/**
	 * @tparam T The target type.
	 * @return The value of a given type T.
	 *
	 * @throws std::bad_variant_access iff this object does not encapsulate
	 * a value of type T.
	 */
	template<typename T>
	T to() const {
		return std::get<T>(m_value);
	}

private :
	/**
	 * Raw type of a possible value for a given Type.
	 */
	using RawValue = std::variant<
			std::string,                 // StringType
			int,                         // IntegerType
			bool,                        // BooleanType
			std::vector<std::string>,    // RegexType
			std::list<Value>,            // ListType
			std::vector<Value>           // TupleType
	>;

	std::shared_ptr<Type const> m_type;
	RawValue m_value;
};

/**
 * Visits a polymorphic value.
 */
class ValueVisitor {
public:
	/**
	 * Handles an unknown value.
	 */
	virtual void handleUnknown() = 0;

	/**
	 * Handles a string value.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleString(std::string const &value) = 0;

	/**
	 * Handles an integer value.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleInteger(int value) = 0;

	/**
	 * Handles a boolean value.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleBoolean(bool value) = 0;

	/**
	 * Handles the result of matching a regular expression.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleMatchedGroups(std::vector<std::string> const &value) = 0;

	/**
	 * Handles a list of values.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleList(std::list<Value> const &value) = 0;

	/**
	 * Handles a tuple of values.
	 *
	 * @param value The value to handle.
	 */
	virtual void handleTuple(std::vector<Value> const &value) = 0;
};

} // V1_0

} // op
} // fli4ld

#endif
