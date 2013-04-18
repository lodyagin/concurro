// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_HPP_
#define CONCURRO_REVENT_HPP_

#include "REvent.h"
#include "RState.h"

template<class Axis>
REvent<Axis>::REvent(RObjectWithEvents<Axis>* obj_ptr, 
							const char* from, 
							const char* to)
  : UniversalEvent
  	   (
#if 0
       StateMapRepository::instance()
		 . get_map_for_axis(typeid(Axis)) ->
#else
		 RAxis<Axis>::instance().state_map() .
#endif
		 get_transition_id(from, to)
		  ),
	 Event(obj_ptr->create_event((UniversalEvent)*this))
{
  evt_ptr->log_params.set =
	 evt_ptr->log_params.reset = false;
}

template<class Axis>
REvent<Axis>::REvent(RObjectWithEvents<Axis>* obj_ptr, 
							const char* to)
  : UniversalEvent
  	   (
#if 0
		 StateMapRepository::instance()
	    . get_map_for_axis(typeid(Axis)) ->
#else
		 RAxis<Axis>::instance().state_map() .
#endif
		  create_state(to), true
		  ),
	 Event(obj_ptr->create_event((UniversalEvent)*this))
{
  evt_ptr->log_params.set =
	 evt_ptr->log_params.reset = false;
}

#if 0
template<class Axis>
bool REvent<Axis>
//
::wait(int time) const
{
  assert(evt);

#if 0
  if (is_arrival_event()) {
	 const uint32_t obj_state = obj->current_state();
	 const uint32_t arrival_state = UniversalState(*this);

	 // TODO they should store state both with or w/o map
	 // id (no STATE_IDX is actually needed)
	 if (STATE_IDX(obj_state) == STATE_IDX(arrival_state))
		return true; // the object is already in that state
  }
#endif

  // wait untill it be
  return evt->wait(time);
}

template<class Axis>
bool REvent<Axis>
//
::signalled() const
{
  assert(evt);
  return evt->signalled();
}
#endif

#endif
