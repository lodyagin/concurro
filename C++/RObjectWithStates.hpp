// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_HPP_
#define CONCURRO_ROBJECTWITHSTATES_HPP_

#include "RObjectWithStates.h"
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
  const auto it = events.find(ue.global_id());
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
  const bool initial_state = ue == current_event;
#if 0
  // map support for emplace is only in gcc 4.9
  return *events.emplace
	 (std::make_pair
	   (ue.global_id(),
		 Event(SFORMAT(typeid(*this).name() << ":" 
							<< ue.name()), 
				 true, initial_state) // <NB> manual reset
		  )).first;
#else										  
  const auto it = events.find(ue.global_id());
  if (it == events.end()) {
	 return events.insert
		(std::make_pair
		 (ue.global_id(), 
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
	 const auto it = events.find(ev.global_id());
	 if (it != events.end())
		it->second.set();
  }

  { // event on a final destination
	 const UniversalEvent ev(to, true);
	 const auto it = events.find(ev.global_id());
	 if (it != events.end())
		it->second.set();
  }
}

#endif
