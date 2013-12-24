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

#include "RObjectWithStates.h"

namespace curr {

RObjectWithStatesBase::RObjectWithStatesBase()
  : is_frozen(false), is_changing(false)
{}

RObjectWithStatesBase
::RObjectWithStatesBase(RObjectWithStatesBase&& o)
{
  //<NB> no parent
  *this = std::move(o);
}

RObjectWithStatesBase::~RObjectWithStatesBase()
{
  for (auto ce : subscribers_terminals)
	 ce.wait();
}

RObjectWithStatesBase& RObjectWithStatesBase
::operator=(RObjectWithStatesBase&& o)
{
  SCHECK(o.is_frozen);
  is_frozen = true;
  is_changing = false;
  subscribers = std::move(o.subscribers);
  subscribers_terminals =
    std::move(o.subscribers_terminals);
  return *this;
}

void RObjectWithStatesBase
::register_subscriber
  (AbstractObjectWithEvents* sub,
   StateAxis* ax
  )
{
  // A guard
  is_changing = true;
  if (is_frozen) {
	 is_changing = false;
	 THROW_PROGRAM_ERROR;
  }

  assert(sub);
  subscribers.insert(std::make_pair(sub, ax));
  subscribers_terminals.insert
	 (std::move(sub->is_terminal_state()));
  assert(subscribers_terminals.size() <= 
			subscribers.size());
  is_changing = false;
}

void RObjectWithStatesBase
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  // A guard
  is_frozen = true;
  if (is_changing) 
	 THROW_PROGRAM_ERROR;

  for (auto sub : subscribers) 
    dynamic_cast<AbstractObjectWithStates*>
      (sub.first)->state_changed
        (*sub.second, state_ax, object, new_state);
}

}
