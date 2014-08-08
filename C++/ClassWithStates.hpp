/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
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

#include "Logging.h"
#include "ClassWithStates.h"

namespace curr {

// TODO specialization for EmptyStateHook (avoid dynamic
// cast and other parameters).
template<
  class Axis, 
  const ::types::constexpr_string& initial_state,
  class StateHook
>
void ClassWithStates<Axis, initial_state, StateHook>
::TheClass::state_changed
  (StateAxis&, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  StateHook()(this, state_ax, new_state);
}

template<class Axis, const ::types::constexpr_string& initial_state,
         class StateHook>
std::atomic<uint32_t>& 
ClassWithStates<Axis, initial_state, StateHook>
::TheClass::current_state(const StateAxis& ax)
{
  static std::atomic<uint32_t> currentState( 
    RAxis<Axis>::state_map()
    -> create_state(initial_state));
  return currentState;
}

template<class Axis, const ::types::constexpr_string& initial_state,
         class StateHook>
const std::atomic<uint32_t>& 
ClassWithStates<Axis, initial_state, StateHook>
::TheClass::current_state(const StateAxis& ax) const
{
  return const_cast<
    ClassWithStates<Axis, initial_state, StateHook>
    ::TheClass*>
      (this) -> current_state(ax);
}

template<class Axis, const ::types::constexpr_string& initial_state,
         class StateHook>
CompoundEvent 
ClassWithEvents<Axis, initial_state, StateHook>
//
::TheClass::create_event(const StateAxis& ax, const UniversalEvent& ue) const
{
  const UniversalEvent current_event
    (this->current_state(Axis::self()), true);
  const bool is_initial_state = 
    ue.local_id() == current_event.local_id();
		  
  const auto it = get_events().find(ue.local_id());
  if (it == get_events().end()) {
    return CompoundEvent(
      get_events().insert
      (std::make_pair
       (ue.local_id(), 
         Event(SFORMAT(
                    ::types::type<decltype(*this)>::name() 
                      << ":" 
                      << ue.name()), 
              true, is_initial_state))).first->second);
  }
  else 
    return CompoundEvent(it->second);
}

template<class Axis, const ::types::constexpr_string& initial_state,
         class StateHook>
void ClassWithEvents<Axis, initial_state, StateHook>
::TheClass::update_events
  (StateAxis& ax, 
   TransitionId trans_id, 
   uint32_t to)
{
  assert(is_same_axis<Axis>(ax));

  /* prevent an infinite loop
  LOG_TRACE(Logger<LOG::Root>, 
            "update_events for the axis "
            << ::types::type<StateAxis>::name()
            << ", to = " << UniversalState(to).name()
            << std::hex << " (0x" << to << ")");
  */

  // reset all events due to a new transition
  for (auto& p : get_events()) p.second.reset();

  { // event on a transition
    const UniversalEvent ev(trans_id);
    const auto it = get_events().find(ev.local_id());
    if (it != get_events().end())
      it->second.set();
  }

  { // event on a final destination
    const UniversalEvent ev(to, true);
    const auto it = get_events().find(ev.local_id());
    if (it != get_events().end())
      it->second.set();
  }
}


}
#endif

