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

#include "StdAfx.h"
#include "RSocket.hpp"
#include "REvent.hpp"
#include "RState.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace curr {

/*=================================*/
/*========== RSocketBase ==========*/
/*=================================*/

DEFINE_AXIS(
  SocketBaseAxis,
  {
      "created",
      "io_ready",
      "bound",
      "closed",
      "connection_timed_out",
      "connection_refused",
      "destination_unreachable",
      "address_already_in_use"
      },
  { {"created", "io_ready"},
    {"created", "bound"},
    {"created", "closed"},
    {"created", "connection_timed_out"},
    {"created", "connection_refused"},
    {"created", "destination_unreachable"},
    {"created", "address_already_in_use"},
    {"bound",  "closed"},
    {"io_ready", "closed"},
    {"closed", "closed"},
  }
  );

DEFINE_STATES(SocketBaseAxis);

DEFINE_STATE_CONST(RSocketBase, State, created);
DEFINE_STATE_CONST(RSocketBase, State, io_ready);
DEFINE_STATE_CONST(RSocketBase, State, bound);
DEFINE_STATE_CONST(RSocketBase, State, closed);
DEFINE_STATE_CONST(RSocketBase, State, 
             connection_timed_out);
DEFINE_STATE_CONST(RSocketBase, State, 
             connection_refused);
DEFINE_STATE_CONST(RSocketBase, State, 
             destination_unreachable);
DEFINE_STATE_CONST(RSocketBase, State, 
             address_already_in_use);

//! This type is only for repository creation
RSocketBase::RSocketBase(const ObjectCreationInfo& oi,
                 const RSocketAddress& addr) 
  : StdIdMember(SFORMAT(addr.get_fd())),
   RObjectWithEvents(createdState),
   CONSTRUCT_EVENT(io_ready),
   CONSTRUCT_EVENT(bound),
   CONSTRUCT_EVENT(closed),
   CONSTRUCT_EVENT(connection_timed_out),
   CONSTRUCT_EVENT(connection_refused),
   CONSTRUCT_EVENT(destination_unreachable),
   CONSTRUCT_EVENT(address_already_in_use),
   fd(addr.get_fd()), 
   is_construction_complete_event
    ("RSocketBase::construction_complete", true),
   repository(dynamic_cast<RSocketRepository*>
          (oi.repository)),
   is_terminal_state_event {
    is_connection_timed_out_event,
    is_connection_refused_event,
    is_destination_unreachable_event,
    is_address_already_in_use_event,
    is_closed_event  
   },
   address(addr)
{
  assert(fd >= 0);
  assert(repository);
  assert(address.get_aw_ptr()->begin()->ai_addr);

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
  size_t max_input_packet,
  RThreadFactory *const tf
) : 
  Parent(id, reserved),
  thread_factory(tf),
  connect_timeout{0},
  pending_connections_queue_size(128),
  use_connect_timeout(false),
  max_input_packet_size(max_input_packet)
{
  assert(thread_factory);
  assert(max_input_packet > 0);
}

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

}
