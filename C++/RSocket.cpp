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

/*=================================*/
/*========== RSocketBase ==========*/
/*=================================*/

//! This type is only for repository creation
RSocketBase::RSocketBase(const RSocketAddress& addr) 
  : StdIdMember(SFORMAT(addr.get_fd())),
	 fd(addr.get_fd()), 
	 aw_ptr(addr.get_aw_ptr());
{
  assert(fd >= 0);
  assert(aw_ptr);
  assert(aw_ptr->ai_addr);

  set_blocking(false);
}

void RSocketBase::set_blocking (bool blocking)
{
#ifdef _WIN32
  u_long mode = (blocking) ? 0 : 1;
  ioctlsocket(socket, FIONBIO, &mode);
#else
  int opts = 0;
  rSocketCheck((opts = fcntl(socket, F_GETFL)) != -1);
  if (blocking)
	 opts &= (~O_NONBLOCK);
  else
	 opts |= O_NONBLOCK;
  rSocketCheck(fcntl(socket, F_SETFL, opts) != -1);
#endif
}

std::ostream&
operator<< (std::ostream& out, const RSocketBase& s)
{
  out << "socket(" << s.fd << ')';
  return out;
}

SocketThreadWithPair::SocketThreadWithPair
  (const ObjectCreationInfo& oi, const Par& p) 
  : SocketThread(oi, p), sock_pair({-1}) 
{
  rSocketCheck(
	 ::socketpair(AF_UNIX, SOCK_DGRAM, sock_pair) == 0);
}

