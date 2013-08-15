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
   ev.log_params().log_obj = obj_ptr;
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
   ev.log_params().log_obj = obj_ptr;
  }
}

#endif
