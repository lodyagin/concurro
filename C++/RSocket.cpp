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

DEFINE_STATES(SocketBaseAxis, 
  {
  "created",
  "ready",
  "closed",
  "error"
  },
  { {"created", "ready"},
	 {"created", "closed"},
    {"created", "error"},
    {"ready", "error"},
	 {"ready", "closed"},
	 {"closed", "closed"},
	 {"closed", "error"}
  });

DEFINE_STATE_CONST(RSocketBase, State, created);
DEFINE_STATE_CONST(RSocketBase, State, ready);
DEFINE_STATE_CONST(RSocketBase, State, error);
DEFINE_STATE_CONST(RSocketBase, State, closed);

//! This type is only for repository creation
RSocketBase::RSocketBase(const ObjectCreationInfo& oi,
								 const RSocketAddress& addr) 
  : StdIdMember(SFORMAT(addr.get_fd())),
	 RObjectWithEvents(createdState),
	 CONSTRUCT_EVENT(ready),
	 CONSTRUCT_EVENT(closed),
	 CONSTRUCT_EVENT(error),
	 fd(addr.get_fd()), 
	 is_construction_complete_event
		("RSocketBase::construction_complete", true),
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

void RSocketBase::process_error(int error)
{
  LOG_INFO(log, "Socket error: " << rErrorMsg(error));
  State::move_to(*this, errorState);
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

SocketThreadWithPair::~SocketThreadWithPair()
{
  rSocketCheck(::close(sock_pair[ForSelect]) == 0);
  rSocketCheck(::close(sock_pair[ForNotify]) == 0);
}
