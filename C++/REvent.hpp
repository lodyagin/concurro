// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_HPP_
#define CONCURRO_REVENT_HPP_

#include "REvent.h"
#include "RState.hpp"

template<class Axis>
REvent<Axis>::REvent(const char* from, 
							const char* to)
  : UniversalEvent
  	   (StateMapRepository::instance()
		 . get_map_for_axis(typeid(Axis))
		 -> get_transition_id(from, to)
		  )
{}

template<class Axis>
REvent<Axis>::REvent(const char* to)
  : UniversalEvent
  	   (StateMapRepository::instance()
	    . get_map_for_axis(typeid(Axis))
		 -> create_state(to), true
		  )
{}

#if 0
template<class Axis>
Event& REvent<Axis>
//
::event(RObjectWithEvents<Axis>& obj)
{
  return *obj.get_event(*this);
}

template<class Axis>
const Event& REvent<Axis>
//
::event(const RObjectWithEvents<Axis>& obj) const
{
  return *obj.get_event(*this);
}
#endif

template<class Axis>
bool REvent<Axis>
//
::wait(const RObjectWithEvents<Axis>& obj, int time) const
{
  const Event* ev = obj.create_event(*this);

  if (is_arrival_event()) {
	 const uint32_t obj_state = obj.current_state();
	 const uint32_t arrival_state = UniversalState(*this);

	 // TODO they should store state both with or w/o map
	 // id (no STATE_IDX is actually needed)
	 if (STATE_IDX(obj_state) == STATE_IDX(arrival_state))
		return true; // the object is already in that state
  }
  // wait untill it be
  return ev->wait(time);
}

#endif
