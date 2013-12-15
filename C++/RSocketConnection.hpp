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

#ifndef CONCURRO_RSOCKETCONNECTION_HPP_
#define CONCURRO_RSOCKETCONNECTION_HPP_

#include "RSocketConnection.h"
#include "RSocketAddress.hpp"
#include "RObjectWithThreads.hpp"

namespace curr {

#if 0
template<NetworkProtocol proto, IPVer ip_ver>
RSocketConnection::InetClientPar<proto, ip_ver>
//
::InetClientPar
  (const std::string& a_host, uint16_t a_port) 
  : host(a_host), port(a_port)
{
  sar->create_addresses
    <SocketSide::Client, proto, ip_ver> (host, port);
}
#endif

template<class Connection>
RServerConnectionFactory<Connection>
//
::RServerConnectionFactory
  (ListeningSocket* l_sock, size_t reserved)
: 
  RStateSplitter
    <ServerConnectionFactoryAxis, ListeningSocketAxis>
      (l_sock, ListeningSocket::boundState),
  RConnectionRepository
    ( typeid(*this).name(), 
      reserved,
      &StdThreadRepository::instance()),
  threads(this),
  lstn_sock(l_sock)
{
  assert(lstn_sock);
  const bool isBound = state_is
    <ListeningSocket, ListeningSocketAxis>
      (*lstn_sock, RSocketBase::boundState);
  SCHECK(isBound);
}

template<class Connection>
RServerConnectionFactory<Connection>::Threads
//
::Threads(RServerConnectionFactory* o) :
  RObjectWithThreads<Threads>
  { 
    new typename RServerConnectionFactory<Connection>
      ::ListenThread::Par() 
  },
  obj(o)
{
}

template<class Connection>
void RServerConnectionFactory<Connection>
//
::ListenThread::run()
{
  RServerConnectionFactory* fact = this->object->obj;
  ListeningSocket* lstn_sock = fact->lstn_sock;

  assert(lstn_sock);
  lstn_sock->ask_listen();
  for (;;) {

    ( lstn_sock->is_accepted() 
    | this->isStopRequested
    ).wait();

    if (this->isStopRequested.signalled())
      break;

    RSocketBase* sock = lstn_sock->get_accepted();
    this->object->obj->create_object
      (typename Connection::ServerPar(sock));
  }
}

}

#endif

