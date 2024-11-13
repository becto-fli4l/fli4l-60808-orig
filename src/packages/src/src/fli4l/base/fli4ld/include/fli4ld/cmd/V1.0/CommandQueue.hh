/*****************************************************************************
 *  @file include/fli4ld/cmd/V1.0/CommandQueue.hh
 *  fli4ld: Command queue (V1.0).
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

#ifndef FLI4LD_CMD_V1_0_COMMAND_QUEUE_H_
#define FLI4LD_CMD_V1_0_COMMAND_QUEUE_H_

#include <fli4ld/cmd/V1.0/Command.hh>
#include <fli4ld/mt/V1.0/Mutex.hh>
#include <fli4ld/mt/V1.0/Semaphore.hh>
#include <memory>
#include <deque>

namespace fli4ld {
namespace cmd {

namespace V1_0 {

/**
 * Represents a thread-safe command queue.
 */
class CommandQueue {
public :
	/**
	 * Appends a command to the queue. This operation never blocks for an
	 * indefinite amount of time.
	 *
	 * @param cmd The command to queue.
	 */
	void queue(std::shared_ptr<Command> cmd);

	/**
	 * Removes the first command out of the queue and returns it. If no command
	 * is available, the calling thread blocks until a command is available.
	 *
	 * @return The command.
	 */
	std::shared_ptr<Command> dequeue();

	/**
	 * Checks whether the queue is empty. Note that the result is only
	 * meaningful if the caller can prevent commands to be queued in parallel.
	 *
	 * @return True if the queue is empty, else false.
	 */
	bool isEmpty() const noexcept;

private :
	using Semaphore = mt::V1_0::Semaphore;
	using Mutex = mt::V1_0::Mutex;

	/// A queue of commands.
	typedef std::deque<std::shared_ptr<Command>> Queue;

	/// Semaphore counting elements in the queue.
	Semaphore m_sem;
	/// Protects the queue.
	mutable Mutex m_queueMutex;
	/// The queue.
	Queue m_queue;
};

} // V1_0

} // cmd
} // fli4ld

#endif
