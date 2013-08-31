/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_H_
#define CONCURRO_REVENT_H_

#include "StateMap.h"
#include "RObjectWithStates.hpp"
#include "Event.h"
#include <set>

namespace curr {

template<class Axis, class Axis2>
class RMixedEvent
: public Axis,
  public UniversalEvent,
  public CompoundEvent
{
public:
  //! Create a from->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
				  const char* from, const char* to);

  //! Create a *->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
				  const char* to);
private:
  typedef Logger<LOG::Events> log;
};

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

#define A_DECLARE_EVENT(axis_, axis_2, event)		\
protected: \
curr::RMixedEvent<axis_, axis_2> is_ ## event ## _event;	\
public: \
  const curr::RMixedEvent<axis_, axis_2>& is_ ## event ()	\
  { return is_ ## event ## _event; } \
private:

#define DECLARE_EVENT(axis_, event) \
  A_DECLARE_EVENT(axis_, axis_, event)

#define CONSTRUCT_EVENT(event)		\
  is_ ## event ## _event(this, #event)

}

#endif 
