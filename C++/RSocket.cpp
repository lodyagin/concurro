// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * Socket types.
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocket.hpp"
#include "REvent.hpp"
#include "RState.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/*=================================*/
/*========== RSocketBase ==========*/
/*=================================*/

DEFINE_AXIS(
  SocketBaseAxis,
  {
    "created",
      "ready",
      "closed",
      "connection_timed_out",
      "connection_refused",
      "destination_unreachable"
      },
  { {"created", "ready"},
    {"created", "closed"},
    {"created", "connection_timed_out"},
    {"created", "connection_refused"},
    {"created", "destination_unreachable"},
    {"ready", "closed"},
    {"closed", "closed"},
  }
  );

DEFINE_STATES(SocketBaseAxis);

DEFINE_STATE_CONST(RSocketBase, State, created);
DEFINE_STATE_CONST(RSocketBase, State, ready);
DEFINE_STATE_CONST(RSocketBase, State, closed);
DEFINE_STATE_CONST(RSocketBase, State, 
						 connection_timed_out);
DEFINE_STATE_CONST(RSocketBase, State, 
						 connection_refused);
DEFINE_STATE_CONST(RSocketBase, State, 
						 destination_unreachable);

//! This type is only for repository creation
RSocketBase::RSocketBase(const ObjectCreationInfo& oi,
								 const RSocketAddress& addr) 
  : StdIdMember(SFORMAT(addr.get_fd())),
	 RObjectWithEvents(createdState),
	 CONSTRUCT_EVENT(ready),
	 CONSTRUCT_EVENT(closed),
	 CONSTRUCT_EVENT(connection_timed_out),
	 CONSTRUCT_EVENT(connection_refused),
	 CONSTRUCT_EVENT(destination_unreachable),
	 fd(addr.get_fd()), 
	 is_construction_complete_event
		("RSocketBase::construction_complete", true),
	 repository(dynamic_cast<RSocketRepository*>
					(oi.repository)),
	 is_terminal_state_event {
		is_connection_timed_out_event,
		is_connection_refused_event,
		is_destination_unreachable_event,
		is_closed_event  
	 },
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

SocketThreadWithPair::~SocketThreadWithPair()
{
  rSocketCheck(::close(sock_pair[ForSelect]) == 0);
  rSocketCheck(::close(sock_pair[ForNotify]) == 0);
}

/*=================================*/
/*======= RSocketRepository =======*/
/*=================================*/

RSocketRepository::RSocketRepository(
  const std::string& id,
  size_t reserved,
  RThreadFactory *const tf
) : 
  Parent(id, reserved),
  thread_factory(tf),
  connect_timeout{0},
  use_connect_timeout(false)
{}

void RSocketRepository::set_connect_timeout_u(int64_t usecs)
{
  constexpr int64_t million = 1000000;
  if (usecs < 0) {
    use_connect_timeout = false;
  }
  else {
    const auto qr = std::div(usecs, million);
    {
      RLOCK(this->objectsM);
      connect_timeout = {qr.quot, qr.rem};
    }
    use_connect_timeout = true;
  }
}

const std::unique_ptr<struct timeval> RSocketRepository
::connect_timeout_timeval() const
{
  RLOCK(this->objectsM);
  return std::unique_ptr<struct timeval>
    (new struct timeval(connect_timeout));
}
