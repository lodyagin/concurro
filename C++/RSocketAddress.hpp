/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKETADDRESS_HPP_
#define CONCURRO_RSOCKETADDRESS_HPP_

#include "RSocketAddress.h"
#include "SException.h"

namespace curr {

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
  if (side == SocketSide::Listening)
    hints.ai_flags |= AI_PASSIVE;
}

template<
  enum SocketSide side,
  enum NetworkProtocol protocol, 
  enum IPVer ip_version
>
std::list<RSocketAddressRepository::GuardType*> 
RSocketAddressRepository
//
::create_addresses(const std::string& host, uint16_t port)
{
  AddressRequest<side,protocol,ip_version> par(host,port);
  return create_several_objects(par);
}

}
#endif
