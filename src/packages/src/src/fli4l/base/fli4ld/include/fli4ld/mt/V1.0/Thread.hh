/*****************************************************************************
 *  @file include/fli4ld/mt/V1.0/Thread.hh
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

#ifndef FLI4LD_MT_V1_0_THREAD_H_
#define FLI4LD_MT_V1_0_THREAD_H_

#include <fli4ld/cmd/V1.0/Command.hh>
#include <memory>
#include <pthread.h>

namespace fli4ld {
namespace mt {

namespace V1_0 {

/**
 * Represents a thread of control.
 */
class Thread final {
	using Command = cmd::V1_0::Command;

public :
	/**
	 * Creates a thread object.
	 *
	 * @param command The underlying command to execute in the new thread
	 * context at thread creation time.
	 */
	Thread(std::shared_ptr<Command> command) :
			m_state(new RunnableState()),
			m_command(command) {
	}

	/**
	 * Destroys the thread object. If the associated thread is running, it is
	 * stopped before destruction.
	 */
	~Thread() {
		stop();
	}

	/**
	 * Starts the thread.
	 * @return
	 *  True if successful, false otherwise.
	 */
	bool start() {
		return m_state->start(*this);
	}

	/**
	 * Waits for the thread to be terminated (a.k.a. "joining" the thread).
	 * This operation does not cancel the thread, use stop() for that.
	 */
	void wait() {
		m_state->wait(*this);
	}

	/**
	 * Cancels the execution of the thread and waits for its termination.
	 */
	void stop() {
		m_state->cancel(*this);
		wait();
	}

private :
	/**
	 * Represents the state of a thread.
	 */
	class State {
	public :
		/// Destructor.
		virtual ~State() {
		}

		/**
		 * Starts the thread.
		 *
		 * @param thread The calling thread object.
		 * @return True if successful, false otherwise.
		 */
		virtual bool start(Thread &thread) = 0;

		/**
		 * Cancels the thread.
		 *
		 * @param thread The calling thread object.
		 */
		virtual void cancel(Thread &thread) = 0;

		/**
		 * Waits for the thread to be terminated.
		 *
		 * @param thread The calling thread object.
		 */
		virtual void wait(Thread &thread) = 0;
	};

	/**
	 * Represents the thread state 'runnable'.
	 */
	class RunnableState final : public State {
	public :
		virtual bool start(Thread &thread) override;
		virtual void cancel(Thread &) override {
			// nothing to do
		}
		virtual void wait(Thread &) override {
			// nothing to do
		}
	};

	/**
	 * Represents the thread state 'running'.
	 */
	class RunningState final : public State {
	public :
		/**
		 * Creates the state object.
		 *
		 * @param threadId The thread identifier of the running thread.
		 */
		RunningState(pthread_t threadId) :
				m_threadId(threadId) {
		}

		virtual bool start(Thread &) override {
			return false;
		}
		virtual void cancel(Thread &thread) override;
		virtual void wait(Thread &thread) override;

	private :
		/// The thread identifier of the running thread.
		pthread_t const m_threadId;
	};

	/// The thread state.
	std::shared_ptr<State> m_state;
	/// The underlying thread command.
	std::shared_ptr<Command> const m_command;

	/**
	 * Changes the thread state.
	 *
	 * @param state The new thread state.
	 */
	void setState(std::shared_ptr<State> state) {
		m_state = state;
	}

	/**
	 * The underlying thread entry point.
	 *
	 * @param arg Not used.
	 */
	static void *entryPoint(void *arg);

};

} // V1_0

} // mt
} // fli4ld

#endif
