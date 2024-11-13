/*****************************************************************************
 *  @file src/fli4ld/mt/V1.0/Semaphore.cc
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

#include <fli4ld/mt/V1.0/Semaphore.hh>

namespace fli4ld {
namespace mt {

namespace V1_0 {

void
Semaphore::down()
{
    Mutex::Lock lock (m_mutex);
    --m_counter;
    while (m_counter < 0) {
        m_cond.wait(lock);
    }
}

void
Semaphore::up()
{
    Mutex::Lock lock (m_mutex);
    ++m_counter;
    if (m_counter >= 0) {
        m_cond.broadcast();
    }
}

} // V1_0

} // mt
} // fli4ld
