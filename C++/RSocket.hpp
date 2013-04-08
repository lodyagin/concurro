// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKET_HPP_
#define CONCURRO_RSOCKET_HPP_

#include "RSocket.h"
//#include "InSocket.h"
//#include "OutSocket.h"

// temporary empty definitions
class ClientSocket : public virtual RSocketBase {};
class ServerSocket : public virtual RSocketBase {};
class TCPSocket : public virtual RSocketBase {};

inline RSocketBase* RSocketAllocator0
  (SocketSide side,
   NetworkProtocol protocol,
   IPVer ver,
	const RSocketAddress& addr)
{
  switch(side)
  {
  case SocketSide::Client:
    return RSocketAllocator1
      <ClientSocket> (protocol, ver, addr);
  case SocketSide::Server:
    return RSocketAllocator1
      <ServerSocket>(protocol, ver, addr);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}
    
template<class Side>
inline RSocketBase* RSocketAllocator1
 (NetworkProtocol protocol,
  IPVer ver,
  const RSocketAddress& addr)
{
  switch(protocol)
  {
  case NetworkProtocol::TCP:
    return RSocketAllocator2<Side, TCPSocket>(ver, addr);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class Side, class Protocol>
inline RSocketBase* RSocketAllocator2
  (IPVer ver, const RSocketAddress& addr)
{
  switch(ver)
  {
  case IPVer::v4:
  case IPVer::v6:
  case IPVer::any:
    return RSocketAllocator<Side, Protocol>(addr);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class... Bases>
inline RSocketBase* RSocketAllocator
  (const RSocketAddress& addr)
{
  return new RSocket</*InSocket, OutSocket,*/ Bases...>(addr);
}
    
#endif

