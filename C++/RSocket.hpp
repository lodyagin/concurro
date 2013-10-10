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

#ifndef CONCURRO_RSOCKET_HPP_
#define CONCURRO_RSOCKET_HPP_

#include "RSocket.h"
#include "ClientSocket.h"
#include "TCPSocket.h"
#include "InSocket.h"
#include "OutSocket.h"

namespace curr {

template<class... Bases>
RSocket<Bases...>
//
::RSocket(const ObjectCreationInfo& oi,
          const RSocketAddress& addr)
  : RSocketBase(oi, addr), Bases(oi, addr)...
{
  RSocketBase::is_construction_complete_event.set();
}


template<class... Bases>
RSocket<Bases...>
//
::~RSocket()
{
  RSocketBase::State::compare_and_move
    (*this, 
     { RSocketBase::createdState,
       RSocketBase::readyState 
     },
     RSocketBase::closedState);

  // wait all parts termination
  /*for (auto& te : this->ancestor_terminals)
	 te.wait();*/
  
  // TODO make an array holder instead of this
  // durty temporary solution
  {
    auto rep = dynamic_cast<StdThreadRepository*>
      (this->repository->thread_factory);
    rep->cancel_subthreads();
  }

  // TODO use and-ed events
  for (auto& teh : this->threads_terminals)
    teh.wait();

  //RSocketBase::is_terminal_state_event.set();
  LOG_DEBUG(log, "~RSocket()");
}

/*
template<class... Bases>
void RSocket<Bases...>
//
::ask_close_out()
{
  if (auto* tcp_sock = dynamic_cast<TCPSocket*>(this))
  {
	 tcp_sock->ask_close_out();
  }
}
*/

// temporary empty definitions
class ServerSocket : public virtual RSocketBase {};

inline RSocketBase* RSocketAllocator0
  (SocketSide side,
   NetworkProtocol protocol,
   IPVer ver,
	const ObjectCreationInfo& oi,
	const RSocketAddress& addr)
{
  switch(side)
  {
  case SocketSide::Client:
    return RSocketAllocator1
      <ClientSocket> (protocol, ver, oi, addr);
#if 0
  case SocketSide::Server:
    return RSocketAllocator1
      <ServerSocket>(protocol, ver, oi, addr);
#endif
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}
    
template<class Side>
inline RSocketBase* RSocketAllocator1
 (NetworkProtocol protocol,
  IPVer ver,
  const ObjectCreationInfo& oi,
  const RSocketAddress& addr)
{
  switch(protocol)
  {
  case NetworkProtocol::TCP:
    return RSocketAllocator2<Side, TCPSocket>
		(ver, oi, addr);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class Side, class Protocol>
inline RSocketBase* RSocketAllocator2
  (IPVer ver, 
	const ObjectCreationInfo& oi,
	const RSocketAddress& addr)
{
  switch(ver)
  {
  case IPVer::v4:
  case IPVer::v6:
  case IPVer::any:
    return RSocketAllocator<Side, Protocol>(oi, addr);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class... Bases>
inline RSocketBase* RSocketAllocator
  (const ObjectCreationInfo& oi,
   const RSocketAddress& addr)
{
  return new RSocket<InSocket, OutSocket, Bases...>
	 (oi, addr);
}

}    
#endif

