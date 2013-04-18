// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * Socket types.
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocket.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/*=================================*/
/*========== RSocketBase ==========*/
/*=================================*/

/*
DEFINE_STATES(
  RSocketAxis,
  {"working", "terminated"},
  {{"working", "terminated"}}
);
DEFINE_STATE_CONST(RSocketBase, State, working);
DEFINE_STATE_CONST(RSocketBase, State, terminated);
*/

//! This type is only for repository creation
RSocketBase::RSocketBase(const ObjectCreationInfo& oi,
								 const RSocketAddress& addr) 
  : StdIdMember(SFORMAT(addr.get_fd())),
	 fd(addr.get_fd()), 
	 repository(dynamic_cast<RSocketRepository*>
					(oi.repository)),
	 aw_ptr(addr.get_aw_ptr())
{
  assert(fd >= 0);
  assert(repository);
  assert(aw_ptr);
  assert(aw_ptr->begin()->ai_addr);

  set_blocking(false);
}

void RSocketBase::set_blocking (bool blocking)
{
#ifdef _WIN32
  u_long mode = (blocking) ? 0 : 1;
  ioctlsocket(socket, FIONBIO, &mode);
#else
  int opts = 0;
  rSocketCheck((opts = fcntl(fd, F_GETFL)) != -1);
  if (blocking)
	 opts &= (~O_NONBLOCK);
  else
	 opts |= O_NONBLOCK;
  rSocketCheck(fcntl(fd, F_SETFL, opts) != -1);
#endif
}

std::ostream&
operator<< (std::ostream& out, const RSocketBase& s)
{
  out << "socket[fd=" << s.fd << ']';
  return out;
}

SocketThreadWithPair::SocketThreadWithPair
  (const ObjectCreationInfo& oi, const Par& p) 
  : SocketThread(oi, p), sock_pair{-1}
{
  rSocketCheck(
	 ::socketpair(AF_LOCAL, SOCK_DGRAM, 0, sock_pair) == 0);
}

