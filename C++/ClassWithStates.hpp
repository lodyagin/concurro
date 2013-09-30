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

#ifndef CONCURRO_CLASSWITHSTATES_HPP_
#define CONCURRO_CLASSWITHSTATES_HPP_

#include "ClassWithStates.h"

namespace curr {

// TODO specialization for EmptyStateHook (avoid dynamic
// cast and other parameters).
template<class T, class Axis, const char* initial_state,
         class StateHook>
void ClassWithStates<T, Axis, initial_state, StateHook>
::TheClass::state_changed
  (StateAxis&, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  StateHook(this->instance)(this, state_ax, new_state);
}

template<class T, class Axis, const char* initial_state,
         class StateHook>
std::atomic<uint32_t>& 
ClassWithStates<T, Axis, initial_state, StateHook>
::TheClass::current_state(const StateAxis& ax)
{
  static std::atomic<uint32_t> currentState( 
    RAxis<Axis>::state_map()
    -> create_state(initial_state));
  return currentState;
}

template<class T, class Axis, const char* initial_state,
         class StateHook>
const std::atomic<uint32_t>& 
ClassWithStates<T, Axis, initial_state, StateHook>
::TheClass::current_state(const StateAxis& ax) const
{
  return const_cast<
    ClassWithStates<T, Axis, initial_state, StateHook>
    ::TheClass*>
      (this) -> current_state(ax);
}

}
#endif

