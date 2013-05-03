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
Event RObjectWithEvents<Axis>
//
::get_event_impl(const UniversalEvent& ue) const
{
  const auto it = events.find(ue.local_id());
  if (it == events.end())
	 THROW_EXCEPTION(REventIsUnregistered, ue);

  return it->second;
}

template<class Axis>
Event RObjectWithEvents<Axis>
//
::create_event(const UniversalEvent& ue) const
{
  const UniversalEvent current_event
	 (this->current_state(), true);
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
	 return events.insert
		(std::make_pair
		 (ue.local_id(), 
		  Event(SFORMAT(typeid(*this).name() << ":" 
							 << ue.name()), 
				  true, initial_state))).first->second;
  }
  else 
	 return it->second;
#endif
}

template<class Axis>
void RObjectWithEvents<Axis>
//
::update_events(TransitionId trans_id, uint32_t to)
{
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
   const State& initial_state)
  : RObjectWithEvents<DerivedAxis>(initial_state),
	 delegate(a_delegate),
	 split_state_id(StateMapInstance<SplitAxis>
						 ::stateMap -> size()),
	 split_transition_id(
		StateMapInstance<SplitAxis>
		::stateMap -> get_max_transition_id())
{
  delegate->register_subscriber(this);
}

#if 0
template<class DerivedAxis, class SplitAxis>
std::atomic<uint32_t>& 
RStateSplitter<DerivedAxis, SplitAxis>
::current_state()
{
}

template<class DerivedAxis, class SplitAxis>
const std::atomic<uint32_t>&
RStateSplitter<DerivedAxis, SplitAxis>
::current_state() const
{
}
#endif

template<class DerivedAxis, class SplitAxis>
Event RStateSplitter<DerivedAxis, SplitAxis>
::get_event(const UniversalEvent& ue)
{
  if (!is_this_event_store(ue))
	 return delegate->get_event(ue);
  else
	 return this->RObjectWithEvents<DerivedAxis>
		::get_event(ue);
}

template<class DerivedAxis, class SplitAxis>
Event RStateSplitter<DerivedAxis, SplitAxis>
::get_event (const UniversalEvent& ue) const
{
  if (!is_this_event_store(ue))
	 return delegate->get_event(ue);
  else
	 return this->RObjectWithEvents<DerivedAxis>
		::get_event(ue);
}

template<class DerivedAxis, class SplitAxis>
Event RStateSplitter<DerivedAxis, SplitAxis>
::create_event (const UniversalEvent& ue) const
{
  if (!is_this_event_store(ue))
	 return delegate->create_event(ue);
  else
	 return this->RObjectWithEvents<DerivedAxis>
		::create_event(ue);
}

template<class DerivedAxis, class SplitAxis>
void RStateSplitter<DerivedAxis, SplitAxis>
::update_events(TransitionId trans_id, uint32_t to) 
{
  if (!is_this_event_store(UniversalEvent(trans_id)))
	 delegate->update_events(trans_id, to);
  else
	 return this->RObjectWithEvents<DerivedAxis>
		::update_events (trans_id, to);
}

template<class DerivedAxis, class SplitAxis>
bool RStateSplitter<DerivedAxis, SplitAxis>
::is_this_event_store(const UniversalEvent& ue) const
{
  if (ue.is_arrival_event()) 
	 return ue.as_state_of_arrival() > split_state_id;
  else 
	 return ue.as_transition_id() > split_transition_id;
}

#endif
