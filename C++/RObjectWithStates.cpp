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

void StateListener
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state)
{
  ax.state_changed
    (this, object, state_ax, new_state);
}

StateTransmitterBase::StateTransmitterBase()
  : is_frozen(false), is_changing(false)
{}

StateTransmitterBase
::StateTransmitterBase(StateTransmitterBase&& o)
{
  //<NB> no parent
  *this = std::move(o);
}

StateTransmitterBase::~StateTransmitterBase()
{
  for (auto ce : subscribers_terminals)
	 ce.wait();
}

StateTransmitterBase& StateTransmitterBase
::operator=(StateTransmitterBase&& o)
{
  SCHECK(o.is_frozen);
  is_frozen = true;
  is_changing = false;
  subscribers = std::move(o.subscribers);
  subscribers_terminals =
    std::move(o.subscribers_terminals);
  return *this;
}

void StateTransmitterBase
::register_subscriber
  (StateListener* sub,
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
    (std::move
     (dynamic_cast<AbstractObjectWithEvents*>(sub)
      ->is_terminal_state()));
  assert(subscribers_terminals.size() <= 
			subscribers.size());
  is_changing = false;
}

}
