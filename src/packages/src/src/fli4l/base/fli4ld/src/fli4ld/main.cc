/*****************************************************************************
 *  @file src/fli4ld/main.cc
 *  fli4ld: Main entry point.
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

#include "daemon/PluginManager.hh"
#include "daemon/V1.0/DaemonImpl.hh"
#include <fli4ld/VersionRange.hh>
#include <iostream>
#include <ostream>
#include <ctime>

using namespace fli4ld::daemon;
using namespace fli4ld::daemon::V1_0;
using namespace fli4ld::cmd::V1_0;
using namespace fli4ld::plugin;
using namespace std;

/**
 * Test command printing a value and waiting a little bit.
 */
class TestCommand : public Command {
public :
	/**
	 * Constructor.
	 *
	 * @param fli4ld The fli4ld API object.
	 * @param number The number to print and the number of µs to wait.
	 */
	TestCommand(Fli4ld &fli4ld, unsigned number) :
			m_fli4ld(fli4ld),
			m_number(number) {
	}

	/**
	 * @return The number set during creation.
	 */
	unsigned getNumber() const noexcept {
		return m_number;
	}

	/**
	 * @copydoc fli4ld::cmd::V1_0::Command::run()
	 *
	 * This method tries to submit a CleanupCommand as well as to resubmit this
	 * command. While the first activity always suceeds, the second one will
	 * fail if the command processor is about to be terminated.
	 */
	virtual void run() override;

private :
	/// The fli4ld API object.
	Fli4ld &m_fli4ld;
	/// The number set during creation.
	unsigned m_number;
};

/**
 * Command simulating cleanup.
 */
class CleanupCommand : public Command {
public :
	/**
	 * Constructor.
	 *
	 * @param cause The TestCommand creating us.
	 */
	CleanupCommand(shared_ptr<TestCommand> cause) :
			m_cause(cause) {
	}

	virtual void run() override {
		cout << '~' << m_cause->getNumber() << endl;
	}

private :
	/// The cause for this cleanup command.
	shared_ptr<TestCommand> m_cause;
};

void TestCommand::run() {
	cout << m_number << endl;

	if (m_fli4ld.invokeLater(shared_ptr<CleanupCommand>(new CleanupCommand(
				static_pointer_cast<TestCommand>(shared_from_this())
			)), true)) {
		cout << "+C" << m_number << endl;
	} else {
		cout << "-C" << m_number << endl;
	}

	if (m_fli4ld.invokeLater(shared_from_this())) {
		cout << "+R" << m_number << endl;
	} else {
		cout << "-R" << m_number << endl;
	}

	struct timespec ts;
	ts.tv_sec  = m_number / 1000;
	ts.tv_nsec = (m_number % 1000) * 1000 * 1000;
	nanosleep(&ts, NULL);
}

/**
 * Main entry point.
 *
 * @param argc The argument count.
 * @param argv The argument array.
 * @return Exit code.
 */
int main(int argc, char *argv[])
{
	if (argc < 2) {
		return 1;
	}

	Fli4ldImpl fli4ld;
	cout
		<< "fli4ld supports plug-in API versions in the range "
		<< fli4ld::VersionRange(PluginMin::InterfaceVersion,
				PluginMax::InterfaceVersion) << "\n"
		<< "fli4ld provides fli4ld API version: "
		<< fli4ld.getAPIVersion()
		<< endl;

	PluginManager loader(fli4ld);
	try {
		shared_ptr<PluginMin> module = loader.load(argv[1]);
		cout
			<< "Plug-in " << argv[1] << " loaded:\n"
			<< "  Name:    " << module->getPluginName() << "\n"
			<< "  Version: " << module->getPluginVersion() << "\n"
			<< "  API:     " << module->getAPIVersion() << endl;

		if (fli4ld.startProcessor()) {
			cout << "Command processor started." << endl;
			for (unsigned number = 0; number < 50; ++number) {
				fli4ld.invokeLater(shared_ptr<TestCommand>(new TestCommand(fli4ld, number)));
			}
			cout << "Waiting for termination of command processor..." << endl;
			fli4ld.stopProcessor();
			cout << "Command processor terminated." << endl;
		} else {
			cout << "Starting command processor failed!" << endl;
		}

		return 0;
	} catch (PluginManager::Exception const &e) {
		cout << e.what() << endl;
		return 1;
	}
}
