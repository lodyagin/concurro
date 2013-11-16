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

#ifndef CONCURRO_LISTENINGSOCKET_H_
#define CONCURRO_LISTENINGSOCKET_H_

#include "RSocket.h"

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

DECLARE_AXIS(ListeningSocketAxis, SocketBaseAxis);

/**
  * Just a marker class for a socket which use accept()
  * system call for generating new ServerSocket -s. Really
  * all its functionality is implemented in TCPSocket.
  *
  * @dot
  * digraph {
  *   start [shape = point]; 
  *   closed [shape = doublecircle];
  *   start -> created;
  *   created -> listen;
  *   listen -> accepting;
  *   accepting -> listen;
  *   listen -> closed;
  *   created -> closed;
  *   closed -> closed;
  * }
  * @enddot
  */
class ListeningSocket 
  : virtual public RSocketBase,
    public RStateSplitter
      <ListeningSocketAxis, SocketBaseAxis>
{
public:
  //! @cond
  DECLARE_STATES(ListeningSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, closed);
  //! @endcond

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    THROW_PROGRAM_ERROR;
  }

  void state_hook
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  { 
    return RStateSplitter
      <ListeningSocketAxis,SocketBaseAxis>
        ::current_state(ax);
    }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter
      <ListeningSocketAxis,SocketBaseAxis>
        ::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter
      <ListeningSocketAxis,SocketBaseAxis>
        ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }
protected:
  ListeningSocket
    (const ObjectCreationInfo& oi, 
     const RSocketAddress& par);
};


//! @}

}
#endif
