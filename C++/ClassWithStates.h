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

#ifndef CONCURRO_CLASSWITHSTATES_H_
#define CONCURRO_CLASSWITHSTATES_H_

#include "ObjectWithStatesInterface.h"

namespace curr {

//! @addtogroup states
//! @{

template<class T, class Axis, const char* initial_state,
         class StateHook>
class ClassWithStates
: public ObjectWithStatesInterface<Axis>
{
public:
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override;

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override;

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override;
};

class EmptyStateHook
{
public:
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state)
  {}
};

//! @}

}
#endif


