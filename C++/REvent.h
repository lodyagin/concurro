/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
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

#include <set>
//#include "StateMap.h"
#include "RObjectWithStates.h"
#include "Event.h"

namespace curr {

//! @addtogroup events
//! @{

#define STATE_MAP_MASK 0x7fff0000
#define STATE_MAP_SHIFT 16
#define STATE_IDX_MASK 0x7fff
#define EVENT_IDX_MASK 0xffff
//! Return a state map id by a state.
#define STATE_MAP(state)                                \
  (((state) & STATE_MAP_MASK) >> STATE_MAP_SHIFT)
#define STATE_IDX(state) ((state) & STATE_IDX_MASK)
#define EVENT_IDX(state) ((state) & EVENT_IDX_MASK)

class UniversalEvent
{
public:
  //! arrival events have this bit set
  enum { Mask = 0x8000 };

  //! Exception: Need an arrival event type here
  class NeedArrivalType;

  //! Whether this event is of an 'arrival' and not
  //! 'transitional' type.
  bool is_arrival_event() const { return (id & Mask); }

  //! Transform arrival events to a state. Can throw
  //! NeedArrivalType. 
  operator UniversalState() const;

  bool operator==(UniversalEvent b) const
  {
    return id == b.id;
  }

  bool operator!=(UniversalEvent b) const
  {
    return id != b.id;
  }

  //! Construct a `transitional' type of an event
  UniversalEvent(TransitionId trans_id) : id(trans_id) {}
  //! Construct an `arrival' type of an event
  UniversalEvent(uint32_t state, bool) : id(state|Mask) 
  {
    assert(STATE_MAP(id)); //must contain a state_map part
  }

  //! Both transition and arrival ids without a map.
  uint16_t local_id() const { return EVENT_IDX(id); }

  //! Both transition and arrival ids with a map.
  uint32_t global_id() const { return id; }

  //! A name as "<state>" or "<state>-><state>"
  std::string name() const; 

  uint32_t as_state_of_arrival() const
  { 
    assert (is_arrival_event());
    return id & ~Mask; 
  }

  TransitionId as_transition_id() const
  {
    assert(!is_arrival_event());
    return id;
  }

protected:
  uint32_t   id;
};

std::ostream&
operator<< (std::ostream& out, const UniversalEvent& ue);

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

  //! Create a from->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
              const RState<Axis>& from, 
              const RState<Axis>& to);

  //! Create a *->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
              const char* to);

  //! Create a *->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
              const RState<Axis>& to);

  //! to_state corresponding to the event
  const RState<Axis> to_state;

private:
//  typedef Logger<LOG::Events> log;
};

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

#if 0
class RCompoundEvent : public CompoundEvent
{
public:
  RCompoundEvent(std::initializer_list<UniversalEvent>);

  const std::set<UniversalState> state_set;
};
#endif

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

//! An arrival event for usage in static members
template<class Axis, class Axis2>
struct a_event_fun;

template<class Axis>
using event_fun = a_event_fun<Axis, Axis>;

template<class Axis, class Axis2>
struct a_trans_event_fun;

template<class Axis>
using trans_event_fun = a_trans_event_fun<Axis, Axis>;

#define E(event) is_ ## event ## _event

}

//! @}

#endif 
