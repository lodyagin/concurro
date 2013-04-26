// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketConnection.h"

RSingleSocketConnection::RSingleSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
 : RSocketConnection(oi, par),
   socket(par.socket_rep->create_object(*par.sock_addr))
{}

RSingleSocketConnection::~RSingleSocketConnection()
{
  socket_rep->delete_object(socket, true);
}
