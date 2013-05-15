// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_HPP_
#define CONCURRO_ROBJECTWITHSTATES_HPP_

#include "RObjectWithStates.h"
#include "RState.h"
#include <algorithm>

template<class Axis>
RObjectWithStates<Axis>
//
::RObjectWithStates(const RObjectWithStates& obj)
{
  currentState = obj.currentState.load();
}

template<class Axis>
RObjectWithStates<Axis>& RObjectWithStates<Axis>
//
::operator=(const RObjectWithStates& obj)
{
  currentState = obj.currentState.load();
  return *this;
}

template<class Axis>
void RObjectWithStates<Axis>
//
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object)
{
  assert(is_same_axis<Axis>(ax));
  RObjectWithStatesBase::state_changed
    (ax, state_ax, object);
  if (mcw)
    mcw->call
      (this, object, state_ax, 
       state_ax.bound(object->current_state(state_ax)));
}

template<class Axis>
CompoundEvent RObjectWithEvents<Axis>
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
      Event(SFORMAT(typeid(*this).name() << ":" 
                    << ue.name()), 
            true, initial_state) // <NB> manual reset
       )).first;
#else										  
  const auto it = events.find(ue.local_id());
  if (it == events.end()) {
    return CompoundEvent(events.insert
                         (std::make_pair
                          (ue.local_id(), 
                           Event(SFORMAT(typeid(*this).name() << ":" 
                                         << ue.name()), 
                                 true, initial_state))).first->second);
  }
  else 
    return CompoundEvent(it->second);
#endif
}

template<class Axis>
void RObjectWithEvents<Axis>
//
::update_events(StateAxis& ax, 
                TransitionId trans_id, 
                uint32_t to)
{
  assert(is_same_axis<Axis>(ax));
  LOG_TRACE(Logger<LOG::Root>, "update_events");
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

template<class DerivedAxis, class SplitAxis>
RStateSplitter<DerivedAxis, SplitAxis>::RStateSplitter
  (RObjectWithEvents<SplitAxis>* a_delegate,
   const State& initial_state,
    AMembWrap* mcw)
    : 
    RObjectWithEvents<DerivedAxis>(initial_state, mcw),
    delegate(a_delegate),
    split_state_id(StateMapInstance<SplitAxis>
                   ::stateMap -> get_n_states()),
    split_transition_id(
      StateMapInstance<SplitAxis>
      ::stateMap -> get_max_transition_id()),
    inited(false)
{
}

#if 0
template<class DerivedAxis, class SplitAxis>
Event RStateSplitter<DerivedAxis, SplitAxis>
::get_event(const UniversalEvent& ue)
{
  if (!is_this_event_store(ue))
    return delegate->RObjectWithEvents<SplitAxis>
      ::get_event(ue);
  else
    return this->RObjectWithEvents<DerivedAxis>
      ::get_event(ue);
}

template<class DerivedAxis, class SplitAxis>
Event RStateSplitter<DerivedAxis, SplitAxis>
::get_event (const UniversalEvent& ue) const
{
  if (!is_this_event_store(ue))
    return delegate->RObjectWithEvents<SplitAxis>
      ::get_event(ue);
  else
    return this->RObjectWithEvents<DerivedAxis>
      ::get_event(ue);
}
#endif

template<class DerivedAxis, class SplitAxis>
CompoundEvent RStateSplitter<DerivedAxis, SplitAxis>
::create_event (const UniversalEvent& ue) const
{
  if (!is_this_event_store(ue)) {
    CompoundEvent ce(
      delegate->RObjectWithEvents<SplitAxis>
      ::create_event(ue));

    // Create event in DerivedAxis if it is a part of
    // DerivedAxis transition.
    if (ue.is_arrival_event()
        && StateMapInstance<DerivedAxis>::stateMap
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

template<class DerivedAxis, class SplitAxis>
void RStateSplitter<DerivedAxis, SplitAxis>
::update_events
  (StateAxis& ax, 
   TransitionId trans_id, 
   uint32_t to) 
{
  init();
  LOG_TRACE(Logger<LOG::Root>, "update_events");
  //<NB> always in DerivedAxis (stat is splitted in this
  //case, derived has another events states)
  this->RObjectWithEvents<DerivedAxis>
    ::update_events(DerivedAxis::self(), trans_id, to);
}

template<class DerivedAxis, class SplitAxis>
bool RStateSplitter<DerivedAxis, SplitAxis>
::is_this_event_store(const UniversalEvent& ue) const
{
  init();
  if (ue.is_arrival_event()) 
    return STATE_IDX(ue.as_state_of_arrival()) 
      > split_state_id;
  else 
    return STATE_IDX(ue.as_transition_id()) 
      > split_transition_id;
}

template<class DerivedAxis, class SplitAxis>
void RStateSplitter<DerivedAxis, SplitAxis>
::init() const
{
  if (!inited) {
    inited = true;
    delegate->register_subscriber
      (const_cast<RStateSplitter<DerivedAxis, SplitAxis>*>
       (this), &DerivedAxis::self());
  }
}

#endif
