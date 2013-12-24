/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
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

#ifndef CONCURRO_STATEAXIS_HPP_
#define CONCURRO_STATEAXIS_HPP_

#include "StateAxis.h"
#include "SException.h"

namespace curr {

const std::atomic<uint32_t>& StateAxis::current_state
  (const AbstractObjectWithStates*) const
{
  THROW_NOT_IMPLEMENTED;
}

std::atomic<uint32_t>& StateAxis::current_state
  (AbstractObjectWithStates*) const
{
    THROW_NOT_IMPLEMENTED;
}

void StateAxis::update_events
  (AbstractObjectWithEvents* obj, 
   TransitionId trans_id, 
   uint32_t to)
{
  THROW_NOT_IMPLEMENTED;
}

void StateAxis::state_changed
  (AbstractObjectWithStates* subscriber,
   AbstractObjectWithStates* publisher,
   const StateAxis& state_ax,
   const UniversalState& new_state)
{
  THROW_NOT_IMPLEMENTED;
}



}

#endif
