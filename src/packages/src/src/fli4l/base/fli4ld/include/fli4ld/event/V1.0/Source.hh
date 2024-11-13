/*****************************************************************************
 *  @file include/fli4ld/event/V1.0/Source.hh
 *  Defines an interface for event sources (V1.0).
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

#ifndef FLI4LD_EVENT_V1_0_SOURCE_H_
#define FLI4LD_EVENT_V1_0_SOURCE_H_

#include <fli4ld/event/V1.0/Sink.hh>
#include <fli4ld/mt/V1.0/Mutex.hh>
#include <list>
#include <algorithm>
#include <functional>

namespace fli4ld {
namespace event {

namespace V1_0 {

/**
 * Represents a source of events.
 * @tparam Event The event type.
 */
template<typename Event>
class Source {
public :

	/// Destructor.
	virtual ~Source() {
	}

	/**
	 * Registers an event sink with this source. If the same sink is registered
	 * multiple times, it receives the event multiple times.
	 *
	 * Note that this source refers to its sinks by weak pointers only, so
	 * registering a sink with a source does not extend the sink's lifetime
	 * to the lifetime of the source.
	 *
	 * @param sink The event sink to register.
	 */
	void registerSink(std::shared_ptr<Sink<Event>> const sink) {
		Mutex::Lock lock(m_sinksMutex);
		m_sinks.push_back(sink);
	}

	/**
	 * Unregisters an event sink from this source. If the same sink is
	 * registered multiple times, it has to be unregistered the same number of
	 * times.
	 *
	 * Note that this source refers to its sinks by weak pointers only, so
	 * registering a sink with a source does not extend the sink's lifetime
	 * to the lifetime of the source.
	 *
	 * @param sink The event sink to register.
	 */
	void unregisterSink(std::shared_ptr<Sink<Event>> const sink);

protected :

	/**
	 * Notifies all registered sinks about an event by calling
	 * Sink<Event>::processEvent(event) on each sink. This operation is const
	 * to make it possible to generate events that are independent of this
	 * object's state changes.
	 *
	 * Note that unregistering a sink from another's sink processEvent() method
	 * does not prevent that sink from being notified, even if it has not been
	 * called up to now, as a copy of the sink list is made prior to calling
	 * the sinks.
	 *
	 * @param event The event that occurred.
	 */
	void notifySinks(std::shared_ptr<Event> const event) const;

private :
	/**
	 * The list of registered sinks. This list is mutable as expired weak
	 * pointers are removed on each call to notifySinks(), which is const.
	 */
	mutable std::list<std::weak_ptr<Sink<Event>>> m_sinks;
	/// Mutex protecting the m_sinks list.
	mutable Mutex m_sinksMutex;
};

template<typename Event>
void Source<Event>::unregisterSink(std::shared_ptr<Sink<Event>> const sink) {
	Mutex::Lock lock(m_sinksMutex);
	typename decltype(m_sinks)::iterator it = std::find_if(
		m_sinks.begin(), m_sinks.end(),
		[sink](std::weak_ptr<Sink<Event>> const weakSink) {
			return sink == weakSink.lock();
		}
	);
	if (it != m_sinks.end()) {
		m_sinks.erase(it);
	}
}

template<typename Event>
void Source<Event>::notifySinks(std::shared_ptr<Event> const event) const {
	std::list<std::shared_ptr<Sink<Event>>> sinks;

	{
		Mutex::Lock lock(m_sinksMutex);
		for (
			typename decltype(m_sinks)::const_iterator it = m_sinks.begin();
			it != m_sinks.end();
		) {
			if (std::shared_ptr<Sink<Event>> const sink = it->lock()) {
				sinks.push_back(sink);
				++it;
			} else {
				it = m_sinks.erase(it);
			}
		}
	}

	// call sinks without holding a lock
	std::for_each(
		sinks.begin(), sinks.end(),
		[event](std::shared_ptr<Sink<Event>> const sink) {
			sink->processEvent(event);
		}
	);
}

} // V1_0

} // event
} // fli4ld

#endif
