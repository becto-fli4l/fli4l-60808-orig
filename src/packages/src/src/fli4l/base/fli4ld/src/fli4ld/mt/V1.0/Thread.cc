/*****************************************************************************
 *  @file src/fli4ld/mt/V1.0/Threads.cc
 *  fli4ld: Thread API (V1.0).
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

#include <fli4ld/mt/V1.0/Thread.hh>

using namespace std;

namespace fli4ld {
namespace mt {

namespace V1_0 {

bool
Thread::RunnableState::start(Thread &thread)
{
    pthread_t threadId = 0;
    if (pthread_create(&threadId, NULL, &entryPoint, &thread) == 0) {
        thread.setState(shared_ptr<State>(new RunningState(threadId)));
        return true;
    }
    else {
        return false;
    }
}

void
Thread::RunningState::cancel(Thread &)
{
    pthread_cancel(m_threadId);
}

void
Thread::RunningState::wait(Thread &thread)
{
    pthread_join(m_threadId, NULL);
    thread.setState(shared_ptr<State>(new RunnableState()));
}

void *
Thread::entryPoint(void *arg)
{
    Thread *const self = static_cast<Thread *>(arg);
    self->m_command->run();
    return 0;
}

} // V1_0

} // mt
} // fli4ld
