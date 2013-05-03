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

template<class Axis, class Axis2>
RMixedEvent<Axis, Axis2>
//
::RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
							const char* from, 
							const char* to)
  : UniversalEvent
  	   (
		 StateMapInstance<Axis>::stateMap
		  -> get_transition_id(from, to)
		  ),
	 Event(obj_ptr->create_event((UniversalEvent)*this))
{
  evt_ptr->log_params.set = 
	 evt_ptr->log_params.reset = false;
  // if you tune it tune it also 
  // in the second constructor
}

template<class Axis, class Axis2>
RMixedEvent<Axis, Axis2>
//
::RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
							const char* to)
  : UniversalEvent
  	   (
		 StateMapInstance<Axis>::stateMap
		 -> create_state(to), true
		  ),
	 Event(obj_ptr->create_event((UniversalEvent)*this))
{
  evt_ptr->log_params.set =
	 evt_ptr->log_params.reset = false;
}

#endif
