/*****************************************************************************
 *  @file include/fli4ld/mt/V1.0/Condition.hh
 *  fli4ld: Condition API (V1.0).
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

#ifndef FLI4LD_MT_V1_0_CONDITION_H_
#define FLI4LD_MT_V1_0_CONDITION_H_

#include <fli4ld/mt/V1.0/Mutex.hh>
#include <pthread.h>

namespace fli4ld {
namespace mt {

namespace V1_0 {

/**
 * Represents an object that threads can wait for and that can be signalled.
 */
class Condition final {
public :
	/**
	 * Creates a condition.
	 */
	Condition() {
		pthread_cond_init(&m_rep, NULL);
	}

	/**
	 * Destroys a condition.
	 */
	~Condition() {
		pthread_cond_destroy(&m_rep);
	}

	/**
	 * Signals this condition. If any threads wait on this condition, one of
	 * then is unblocked which can continue execution.
	 */
	void signal() {
		pthread_cond_signal(&m_rep);
	}

	/**
	 * Broadcasts this condition. If any threads wait on this condition, all of
	 * them are unblocked and one of them can continue execution.
	 */
	void broadcast() {
		pthread_cond_broadcast(&m_rep);
	}

	/**
	 * Waits for this condition to be signalled.
	 *
	 * @param lock The mutex locker protecting access to the resource
	 * controlled by the condition.
	 */
	void wait(Mutex::Lock &lock) {
		pthread_cond_wait(&m_rep, &lock.m_mutex.m_rep);
	}

private :
	/// Internal pthreads object.
	pthread_cond_t m_rep;
};

} // V1_0

} // mt
} // fli4ld

#endif
