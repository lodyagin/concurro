// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKETADDRESS_HPP_
#define CONCURRO_RSOCKETADDRESS_HPP_

#include "RSocketAddress.h"
#include "SException.h"


/*==================================*/
/*========== HintsBuilder ==========*/
/*==================================*/

template<IPVer ver> 
IPVerHints<ver>
//
::IPVerHints()
{
  switch(ver)
  {
  case IPVer::v4: 
	 hints.ai_family = AF_INET;
	 break;
  case IPVer::v6: 
	 hints.ai_family = AF_INET6;
	 break;
  case IPVer::any:
	 hints.ai_family = AF_UNSPEC;
	 break;
  default: THROW_NOT_IMPLEMENTED;
  }
}

template<SocketSide side> 
SocketSideHints<side>
//
::SocketSideHints()
{
  if (side == SocketSide::Server)
	 hints.ai_flags |= AI_PASSIVE;
}




template<
  enum NetworkProtocol protocol, 
  enum IPVer ip_version
>
std::list<RSocketAddress*> SocketAddressRepository
//
::create_addresses(const std::string& host, uint16_t port)
{
  AddressRequest<protocol, ip_version> par(host, port);   
  return create_several_objects(par);
}

#endif
