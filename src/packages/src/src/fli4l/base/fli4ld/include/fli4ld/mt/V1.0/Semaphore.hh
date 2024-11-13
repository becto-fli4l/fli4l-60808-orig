/*****************************************************************************
 *  @file include/fli4ld/mt/V1.0/Semaphore.hh
 *  fli4ld: Semaphore API (V1.0).
 *
 *  Copyright (c) 2015 - fli4l-Team <team@fli4l.de>
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

#ifndef FLI4LD_MT_V1_0_SEMAPHORE_H_
#define FLI4LD_MT_V1_0_SEMAPHORE_H_

#include <fli4ld/mt/V1.0/Mutex.hh>
#include <fli4ld/mt/V1.0/Condition.hh>

namespace fli4ld {
namespace mt {

namespace V1_0 {

/**
 * Represents a semaphore, a thread-safe counter.
 */
class Semaphore final {
public :
	/**
	 * Creates the semaphore.
	 *
	 * @param counter The initial value of the semaphore (defaults to zero).
	 */
	Semaphore(int counter = 0) :
			m_counter(counter) {
	}

	/// Increments the semaphore.
	void down();

	/**
	 * Decrements the semaphore. If the semaphore value is zero, the calling
	 * thread is put to sleep until another thread increments the semaphore
	 * again.
	 */
	void up();

private :
	/// The underlying mutex protecting the counter.
	Mutex m_mutex;
	/**
	 * The condition used to communicate between incrementing and decrementing
	 * threads.
	 */
	Condition m_cond;
	/// The underlying counter.
	int m_counter;
};

} // V1_0

} // mt
} // fli4ld

#endif
