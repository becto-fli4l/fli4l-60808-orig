/*****************************************************************************
 *  @file include/fli4ld/mt/V1.0/Mutex.hh
 *  fli4ld: Mutex API (V1.0).
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

#ifndef FLI4LD_MT_V1_0_MUTEX_H_
#define FLI4LD_MT_V1_0_MUTEX_H_

#include <pthread.h>

namespace fli4ld {
namespace mt {

namespace V1_0 {

/**
 * Represents a mutex for protecting a resource by mutual exclusion.
 */
class Mutex final {
public :
	class Lock;

	/// Creates a mutex.
	Mutex() {
		pthread_mutex_init(&m_rep, NULL);
	}

	/// Destroys a mutex.
	~Mutex() {
		pthread_mutex_destroy(&m_rep);
	}

private :
	/// Internal pthreads object.
	pthread_mutex_t m_rep;

	/**
	 * Acquires ownership of the mutex. If another thread owns the mutex, the
	 * calling thread will be put to sleep until the mutex becomes available.
	 */
	void lock() {
		pthread_mutex_lock(&m_rep);
	}

	/**
	 * Releases ownership of the mutex.
	 */
	void unlock() {
		pthread_mutex_unlock(&m_rep);
	}

friend class Condition;
};

/**
 * Mutex locking helper class.
 */
class Mutex::Lock final {
public :
	/**
	 * Acquires ownership of a mutex.
	 *
	 * @param mutex The mutex to lock.
	 */
	Lock(Mutex &mutex) :
			m_mutex(mutex) {
		m_mutex.lock();
	}

	/**
	 * Releases ownership of the mutex locked by the constructor.
	 */
	~Lock() {
		m_mutex.unlock();
	}

private :
	/// The underlying mutex.
	Mutex &m_mutex;

friend class Condition;
};

} // V1_0

} // mt
} // fli4ld

#endif
