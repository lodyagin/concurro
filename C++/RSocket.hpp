// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

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
//#include "InSocket.h"
//#include "OutSocket.h"

template<class... Bases>
RSocket<Bases...>
//
::~RSocket()
{
  // wait all parts termination
  for (auto& te : this->ancestor_terminals)
	 te.wait();
  //RSocketBase::is_terminal_state_event.set();
  LOG_DEBUG(log, "~RSocket()");
}

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
  return new RSocket</*InSocket, OutSocket,*/ Bases...>
	 (oi, addr);
}
    
#endif

