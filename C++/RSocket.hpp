// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKET_HPP_
#define CONCURRO_RSOCKET_HPP_

#include "RSocket.h"
#include "InSocket.h"
#include "OutSocket.h"

// temporary empty definitions
class ClientSocket : public virtual RSocketBase {};
class ServerSocket : public virtual RSocketBase {};
class TCPSocket : public virtual RSocketBase {};

inline RSocketBase* RSocketAllocator0
  (SocketSide side,
   NetworkProtocol protocol,
   IPVer ver)
{
  switch(side)
  {
  case SocketSide::Client:
    return RSocketAllocator1
      <ClientSocket> (protocol, ver);
  case SocketSide::Server:
    return RSocketAllocator1
      <ServerSocket>(protocol, ver);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}
    
template<class Side>
inline RSocketBase* RSocketAllocator1
 (NetworkProtocol protocol,
  IPVer ver)
{
  switch(protocol)
  {
  case NetworkProtocol::TCP:
    return RSocketAllocator2<Side, TCPSocket>(ver);
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class Side, class Protocol>
inline RSocketBase* RSocketAllocator2(IPVer ver)
{
  switch(ver)
  {
  case IPVer::v4:
  case IPVer::v6:
  case IPVer::any:
    return RSocketAllocator<Side, Protocol>();
  default: 
    THROW_NOT_IMPLEMENTED;
  }
}

template<class... Bases>
inline RSocketBase* RSocketAllocator()
{
  return new RSocket<InSocket, OutSocket, Bases...>();
}
    
#endif

