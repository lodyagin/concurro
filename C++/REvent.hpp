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
   // G++-4.7.3 bug when use copy constructor
   CompoundEvent
   (std::move(obj_ptr->create_event(
            (UniversalEvent)*this)))
{
  for (Event ev : *this) {
   ev.log_params().set = 
     ev.log_params().reset = false;
   ev.log_params().logger = 
     Logger<RMixedEvent<Axis, Axis2>>::logger();
   // if you tune it tune it also 
   // in the second constructor
  }
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
   // G++-4.7.3 bug when use copy constructor
   CompoundEvent
   (std::move(obj_ptr->create_event(
            (UniversalEvent)*this)))
{
  for (Event ev : *this) {
   ev.log_params().set = 
     ev.log_params().reset = false;
   ev.log_params().logger = 
     Logger<RMixedEvent<Axis, Axis2>>::logger();
  }
}

#endif
