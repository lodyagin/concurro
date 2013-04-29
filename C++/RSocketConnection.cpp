// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketConnection.h"
#include "OutSocket.h"
#include "ClientSocket.h"
//#include "TCPSocket.h"

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
  //TODO not only TCP
  //dynamic_cast<TCPSocket*>(socket)->ask_close();
  socket->ask_close_out();
  socket_rep->delete_object(socket, true);
}

RSocketConnection& RSingleSocketConnection
::operator<< (const std::string str)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  out_sock->msg.is_discharged().wait();
  out_sock->msg.reserve(str.size());
  ::strncpy((char*)out_sock->msg.data(), str.c_str(),
				str.size());
  out_sock->msg.resize(str.size());
  return *this;
}

RSocketConnection& RSingleSocketConnection
::operator<< (RSingleBuffer&& buf)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  out_sock->msg = std::move(buf);
  return *this;
}

void RSingleSocketConnection::ask_connect()
{
  dynamic_cast<ClientSocket*>(socket)->ask_connect();
}

void RSingleSocketConnection::ask_close()
{
  socket->ask_close_out();
//  (socket->is_closed() | socket->is_error()) . wait();
}
