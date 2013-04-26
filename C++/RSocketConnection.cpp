// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketConnection.h"

std::ostream&
operator<< (std::ostream& out, const RSocketConnection& c)
{
  out << "<connection>";
  return out;
}

RSocketConnection::RSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
  : StdIdMember(oi.objectId),
	 socket_rep(dynamic_cast<RConnectionRepository*>
					(oi.repository)->sock_rep)
{
  assert(socket_rep);
}


RSingleSocketConnection::RSingleSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
 : RSocketConnection(oi, par),
   socket(socket_rep->create_object(*par.sock_addr))
{}

RSingleSocketConnection::~RSingleSocketConnection()
{
  socket_rep->delete_object(socket, true);
}

