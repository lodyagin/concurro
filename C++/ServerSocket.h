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

#ifndef CONCURRO_SERVERSOCKET_H_
#define CONCURRO_SERVERSOCKET_H_

#include "RSocket.h"

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

//DECLARE_AXIS(ServerSocketAxis, SocketBaseAxis);

/**
  * It is a socket created as a result of accept() system
  * call.
  */
class ServerSocket 
  : virtual public RSocketBase//,
//    public RStateSplitter<ServerSocketAxis, SocketBaseAxis>
{
public:
  //! @cond
  //! @endcond

  const CompoundEvent is_terminal_state_event;

  std::string object_name() const override
  {
    return SFORMAT("ServerSocket:" << universal_id());
  }

  ~ServerSocket();

  std::string universal_id() const override
  {
    return RSocketBase::universal_id();
  }

#if 0
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
    return RStateSplitter<ServerSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter<ServerSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter<ServerSocketAxis,SocketBaseAxis>
      ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }
#endif

protected:
  ServerSocket
    (const ObjectCreationInfo& oi, 
     const RSocketAddress& par);

//  void process_error(int error);

  DEFAULT_LOGGER(ServerSocket);
};

//! @}

}
#endif
