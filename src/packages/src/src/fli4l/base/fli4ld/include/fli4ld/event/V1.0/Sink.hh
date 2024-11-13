/*****************************************************************************
 *  @file include/fli4ld/event/V1.0/Sink.hh
 *  Defines an interface for event sinks (V1.0).
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

#ifndef FLI4LD_EVENT_V1_0_SINK_H_
#define FLI4LD_EVENT_V1_0_SINK_H_

#include <memory>

namespace fli4ld {
namespace event {

namespace V1_0 {

template<typename Event> class Source;

/**
 * Handles events being emitted by some source(s).
 * @tparam Event The event type.
 */
template<typename Event>
class Sink {
public :

	/// Public destructor.
	virtual ~Sink() {
	}

protected :

	/**
	 * Requests the sink to process an event. This operation is called by
	 * Source::notifySinks().
	 *
	 * @param event The event to handle.
	 */
	virtual void processEvent(std::shared_ptr<Event> event) = 0;

friend Source<Event>; // needs to call processEvent()
};

} // V1_0

} // event
} // fli4ld

#endif
