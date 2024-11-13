/*****************************************************************************
 *  @file include/fli4ld/cmd/V1.0/CommandProcessor.hh
 *  fli4ld: Command processor (V1.0).
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

#ifndef FLI4LD_CMD_V1_0_COMMAND_PROCESSOR_H_
#define FLI4LD_CMD_V1_0_COMMAND_PROCESSOR_H_

#include <fli4ld/cmd/V1.0/CommandQueue.hh>
#include <fli4ld/mt/V1.0/Mutex.hh>
#include <fli4ld/mt/V1.0/Thread.hh>
#include <memory>

namespace fli4ld {
namespace cmd {

namespace V1_0 {

/**
 * Represents an active object executing commands.
 */
class CommandProcessor
{
public :
	/// Constructor. Does not start the thread.
	CommandProcessor() :
			m_thread(std::shared_ptr<Command>(new Executor(*this))),
			m_terminationRequested(false) {
	}

	/// Destructor.
	virtual ~CommandProcessor() {
		stop();
	}

	/**
	 * Starts the processor.
	 *
	 * @return True if successful, false otherwise.
	 */
	bool start() {
		return m_thread.start();
	}

	/**
	 * Cancels the execution of the processor and waits for its termination by
	 * posting a special quit command to the command queue.
	 */
	void stop();

	/**
	 * Requests the active object to execute passed command. The execution
	 * will take place in the active object's own thread.
	 *
	 * @param cmd The command to execute.
	 * @param isCleanup If true, the command is queued even if the queue is in
	 * the middle of being stopped. This is to allow cleanup procedures to be
	 * split into multiple commands.
	 *
	 * @return True if queuing the command has been successful, else false.
	 * Queuing a command fails if termination of this active object has already
	 * been requested via stop() and "isCleanup" is not true.
	 *
	 * @throws invalid_argument if an empty pointer is passed.
	 */
	bool queue(std::shared_ptr<Command> cmd, bool isCleanup = false);

private :
	class QuitCommand;

	using Mutex = mt::V1_0::Mutex;
	using Thread = mt::V1_0::Thread;

	/**
	 * Handles the execution of commands.
	 */
	class Executor : public Command {
	public :
		/**
		 * Constructor.
		 *
		 * @param processor
		 *  The underlying command processor.
		 */
		Executor(CommandProcessor &processor) :
				m_processor(processor) {
		}

		/**
		 * Executes commands being in the processor's command queue. The method
		 * is itself executed in another thread context.
		 */
		virtual void run() override {
			m_processor.processCommands();
		}

	private :
		/// The command processor.
		CommandProcessor &m_processor;
	};

	/**
	 * Requests termination of the active object. The active object stops its
	 * execution as soon as the command queue is empty. Note that a termination
	 * request has no effect if the commands keep the command queue non-empty.
	 */
	void requestTermination() {
		Mutex::Lock lock(m_terminationMutex);
		m_terminationRequested = true;
	}

	/**
	 * Determines if command processor should terminate. This is the case if
	 * termination has been requested by requestTermination() and the command
	 * queue is empty.
	 *
	 * @return True if processor can terminate, else false.
	 */
	bool shouldTerminate() const;

	/**
	 * Processes all commands in the queue until termination is requested and
	 * the queue is empty.
	 */
	void processCommands();

	/// The command queue.
	CommandQueue m_queue;
	/// The underlying thread.
	Thread m_thread;
	/// Serializes termination activities.
	mutable Mutex m_terminationMutex;
	/// True if termination has been requested.
	bool m_terminationRequested;
};

} // V1_0

} // cmd
} // fli4ld

#endif
