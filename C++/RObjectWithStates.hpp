/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_HPP_
#define CONCURRO_ROBJECTWITHSTATES_HPP_

#include "RObjectWithStates.h"
#include "RState.hpp"
#include <algorithm>

namespace curr {

template<size_t maxs>
RObjectWithStatesBase<maxs>::~RObjectWithStatesBase()
{
  for (auto& s : subscribers)
    s.terminal.wait();
}

template<size_t maxs>
void RObjectWithStatesBase<maxs>
::register_subscriber(
  AbstractObjectWithEvents* sub,
  StateAxis* ax
)
{
  const size_t idx = n_subs++;

  assert(sub);
  assert(ax);

  Subscriber& s = subscribers.at(idx);
  s.object = sub;
  s.axis = ax;
  s.terminal = std::move(sub->is_terminal_state());
  assert(!s.ready);
  s.ready = true;
}

template<size_t maxs>
void RObjectWithStatesBase<maxs>
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  for (auto& s : subscribers) {
    if (!s.ready)
      break;

    dynamic_cast<AbstractObjectWithStates*>
      (s.object)->state_changed
        (*s.axis, state_ax, object, new_state);
  }
}

#if 0
template<class Axis, size_t max_subscribers>
RObjectWithStates<Axis, max_subscribers>
//
::RObjectWithStates(const RObjectWithStates& obj)
  : RObjectWithStatesBase<max_subscribers>(obj),
    currentState(obj.currentState.load())
{}

template<class Axis, size_t max_subscribers>
RObjectWithStates<Axis, max_subscribers>
//
::RObjectWithStates(RObjectWithStates&& obj)
  : RObjectWithStatesBase<max_subscribers>(std::move(obj)),
    currentState(obj.currentState.load())
{}

template<class Axis, size_t max_subscribers>
RObjectWithStates<Axis, max_subscribers>& 
RObjectWithStates<Axis, max_subscribers>
//
::operator=(const RObjectWithStates& obj)
{
  RObjectWithStatesBase<max_subscribers>::operator=(
    static_cast
      <const RObjectWithStatesBase<max_subscribers>&>(obj)
  );
  currentState = obj.currentState.load();
  return *this;
}

template<class Axis, size_t max_subscribers>
RObjectWithStates<Axis, max_subscribers>& 
RObjectWithStates<Axis, max_subscribers>
//
::operator=(RObjectWithStates&& obj)
{
  RObjectWithStatesBase<max_subscribers>::operator=(
    static_cast<RObjectWithStatesBase<max_subscribers>&&>
      (std::move(obj))
  );
  currentState = obj.currentState.load();
  return *this;
}
#endif

template<class Axis, size_t max_subscribers>
void RObjectWithStates<Axis, max_subscribers>
//
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  if (is_same_axis<Axis>(ax))
  {
    RObjectWithStatesBase<max_subscribers>::state_changed
      (ax, state_ax, object, new_state);
    if (mcw)
      mcw->call(this, object, state_ax, new_state);
  }
}

template<class Axis, size_t max_subscribers>
RObjectWithEvents<Axis, max_subscribers>
::RObjectWithEvents(
  const typename RObjectWithStates<Axis, max_subscribers>
    ::State& initial_state,
  AMembWrap* mcw
)
  : Parent(initial_state, mcw)
{}

#if 0
template<class Axis, size_t max_subscribers>
RObjectWithEvents<Axis, max_subscribers>
::RObjectWithEvents(RObjectWithEvents&& o)
  : Parent(std::move(o)), events(std::move(o.events))
{}

template<class Axis, size_t max_subscribers>
RObjectWithEvents<Axis, max_subscribers>& 
RObjectWithEvents<Axis, max_subscribers>
//
::operator= (RObjectWithEvents&& o)
{
  Parent::operator= (static_cast<Parent&&>(std::move(o)));
  events = std::move(o.events);
  return *this;
}
#endif

template<class Axis, size_t max_subscribers>
CompoundEvent RObjectWithEvents<Axis, max_subscribers>
//
::create_event(const UniversalEvent& ue) const
{
  const UniversalEvent current_event
    (this->current_state(Axis::self()), true);
  const bool initial_state = 
    ue.local_id() == current_event.local_id();
#if 0
  // map support for emplace is only in gcc 4.9
  return *events.emplace
    (std::make_pair
     (ue.local_id(),
      Event(SFORMAT(curr::type<*this>::name() << ":" 
                    << ue.name()), 
            true, initial_state) // <NB> manual reset
       )).first;
#else										  
  const auto it = events.find(ue.local_id());
  if (it == events.end()) {
    return CompoundEvent(
      events.insert
      (std::make_pair
       (ue.local_id(), 
        Event(SFORMAT(curr::type<decltype(*this)>::name() 
                      << ":" 
                      << ue.name()), 
              true, initial_state))).first->second);
  }
  else 
    return CompoundEvent(it->second);
#endif
}

template<class Axis, size_t max_subscribers>
void RObjectWithEvents<Axis, max_subscribers>
//
::update_events(StateAxis& ax, 
                TransitionId trans_id, 
                uint32_t to)
{
  assert(is_same_axis<Axis>(ax));

  LOG_TRACE(Logger<LOG::Root>, 
            "update_events for the axis "
            << curr::type<StateAxis>::name()
            << ", to = " << UniversalState(to).name()
            << std::hex << " (0x" << to << ")");

  // reset all events due to a new transition
  for (auto& p : events) p.second.reset();

  { // event on a transition
    const UniversalEvent ev(trans_id);
    const auto it = events.find(ev.local_id());
    if (it != events.end())
      it->second.set();
  }

  { // event on a final destination
    const UniversalEvent ev(to, true);
    const auto it = events.find(ev.local_id());
    if (it != events.end())
      it->second.set();
  }
}

template<class DerivedAxis, class SplitAxis, size_t maxs, size_t maxs_delegate>
RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>::RStateSplitter
  (RObjectWithEvents<SplitAxis, maxs_delegate>* a_delegate,
   const State& initial_state,
    AMembWrap* mcw)
    : 
    RObjectWithEvents<DerivedAxis>(initial_state, mcw),
    delegate(a_delegate),
    split_state_id(StateMapInstance<SplitAxis>
                   ::get_map() -> get_n_states()),
    split_transition_id(
      StateMapInstance<SplitAxis>
      ::get_map() -> get_max_transition_id()),
    inited(false)
{
}

template<class DerivedAxis, class SplitAxis, size_t maxs, size_t maxs_delegate>
CompoundEvent RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>
::create_event (const UniversalEvent& ue) const
{
  if (!is_this_event_store(ue)) {
    CompoundEvent ce(
      delegate->RObjectWithEvents<SplitAxis>
      ::create_event(ue));

    // Create event in DerivedAxis if it is a part of
    // DerivedAxis transition.
    if (ue.is_arrival_event()
        && StateMapInstance<DerivedAxis>::get_map()
        -> is_local_transition_arrival
        (ue.as_state_of_arrival())) 
    {
      ce |= this->RObjectWithEvents<DerivedAxis>
        ::create_event(ue);
    }
    return ce;
  }
  else
    return this->RObjectWithEvents<DerivedAxis>
      ::create_event(ue);
}

template<class DerivedAxis, class SplitAxis, size_t maxs, size_t maxs_delegate>
void RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>
::update_events
  (StateAxis& ax, 
   TransitionId trans_id, 
   uint32_t to) 
{
  assert(inited);
  LOG_TRACE(Logger<LOG::Root>, "update_events");
  //<NB> always in DerivedAxis (stat is splitted in this
  //case, derived has another events states)
  this->RObjectWithEvents<DerivedAxis>
    ::update_events(DerivedAxis::self(), trans_id, to);
}

template<class DerivedAxis, class SplitAxis, size_t maxs, size_t maxs_delegate>
bool RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>
::is_this_event_store(const UniversalEvent& ue) const
{
  //assert(inited);
  if (ue.is_arrival_event()) 
    return STATE_IDX(ue.as_state_of_arrival()) 
      > split_state_id;
  else 
    return STATE_IDX(ue.as_transition_id()) 
      > split_transition_id;
}

template<class DerivedAxis, class SplitAxis, size_t maxs, size_t maxs_delegate>
void RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>
::init() const
{
  if (!inited) {
    inited = true;
    delegate->register_subscriber
      (const_cast<RStateSplitter<DerivedAxis, SplitAxis, maxs, maxs_delegate>*>
       (this), &DerivedAxis::self());
  }
}

}
#endif
