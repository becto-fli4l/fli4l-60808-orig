/*****************************************************************************
 * @file src/fli4ld/cmd/V1.0/CommandProcessor.cc
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

#include <fli4ld/cmd/V1.0/CommandProcessor.hh>
#include <stdexcept>

using namespace std;

namespace fli4ld {
namespace cmd {

namespace V1_0 {

/**
 * Asks a command processor to quit execution.
 */
class CommandProcessor::QuitCommand : public Command {
public :
	/**
	 * Constructor.
	 *
	 * @param processor The command processor to ask to terminate.
	 */
	QuitCommand(CommandProcessor &processor) :
			m_processor(processor) {
	}

	virtual void run() override {
		m_processor.requestTermination();
	}

private :
	CommandProcessor &m_processor; ///< The underlying command processor.
};

void
CommandProcessor::stop()
{
	// ignore result as we don't care whether we are the first to stop this
	// processor
	(void) queue(shared_ptr<Command>(new QuitCommand(*this)));
	m_thread.wait();
}

bool CommandProcessor::queue(std::shared_ptr<Command> cmd, bool isCleanup) {
	if (!cmd) {
		throw invalid_argument("Invalid command passed to CommandProcessor::queue()");
	}

	// lock also embodies call to queue() to make check-and-queue atomic
	Mutex::Lock lock(m_terminationMutex);
	if (!isCleanup && m_terminationRequested) {
		return false;
	}

	m_queue.queue(cmd);
	return true;
}

bool
CommandProcessor::shouldTerminate() const
{
	Mutex::Lock lock(m_terminationMutex);
	return m_terminationRequested && m_queue.isEmpty();
}

void
CommandProcessor::processCommands()
{
	while (!shouldTerminate()) {
		m_queue.dequeue()->run();
	}
}

} // V1_0

} // cmd
} // fli4ld
