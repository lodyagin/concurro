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

#include "StdAfx.h"
#include "OutSocket.h"
//#include "REvent.hpp"
#include "RState.hpp"

namespace curr {

/*
DEFINE_STATES(OutSocketAxis);

DEFINE_STATE_CONST(OutSocket, State, wait_you);
DEFINE_STATE_CONST(OutSocket, State, busy);
DEFINE_STATE_CONST(OutSocket, State, closed);
DEFINE_STATE_CONST(OutSocket, State, error);
*/

OutSocket::OutSocket
  (const ObjectCreationInfo& oi, 
	const RSocketAddress& par)
: 
	 RSocketBase(oi, par),
	 /*RObjectWithEvents<OutSocketAxis>(wait_youState),
	 CONSTRUCT_EVENT(wait_you),
	 CONSTRUCT_EVENT(error),
	 CONSTRUCT_EVENT(closed),*/
	 select_thread(dynamic_cast<SelectThread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(SelectThread::Par(this)))),
	 wait_thread(
		dynamic_cast<WaitThread*>
		(RSocketBase::repository->thread_factory
		 -> create_thread
		 (WaitThread::Par
		  (this, 
			select_thread->get_notify_fd()))))
{
  SCHECK(select_thread && wait_thread);
  /*this->RSocketBase::ancestor_terminals.push_back
	 (is_terminal_state());*/
  this->RSocketBase::threads_terminals.push_back
	 (select_thread->is_terminal_state());
  this->RSocketBase::threads_terminals.push_back
	 (wait_thread->is_terminal_state());
  select_thread->start();
  wait_thread->start();
}

OutSocket::~OutSocket()
{
  LOG_DEBUG(log, "~OutSocket()");
}

void OutSocket::SelectThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  auto* out_sock = dynamic_cast<OutSocket*>
	 (socket);
  SCHECK(out_sock);

  ( socket->is_io_ready()
  | socket->is_terminal_state()
  ) . wait();

  ( out_sock->msg.is_charged()
	 | socket->is_terminal_state()
  ) . wait();

  if (socket->is_terminal_state().signalled())
	 return;

  fd_set wfds, rfds;
  FD_ZERO(&wfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  for(;;) {
    // Wait for buffer space
    if (RSocketBase::State::state_is
        (*socket, RSocketBase::io_readyState))
      FD_SET(fd, &wfds);
    else
      FD_CLR(fd, &wfds);
    // The second socket for close report
    FD_SET(sock_pair[ForSelect], &rfds);
    const int maxfd = std::max(sock_pair[ForSelect], fd)
      + 1;
    rSocketCheck(
      ::select(maxfd, &rfds, &wfds, NULL, NULL) > 0);
    LOG_DEBUG(OutSocket::log, "OutSocket>\t ::select");

    if (FD_ISSET(fd, &wfds)) {
      const ssize_t written = 
        ::write(fd, out_sock->msg.cdata(),
                out_sock->msg.size());
      if (written < 0) {
        const int err = errno;
        LOG_ERROR(OutSocket::log, "Error " << rErrorMsg(err));
      }

      assert(written <= (ssize_t) out_sock->msg.size());
      if (written < (ssize_t) out_sock->msg.size())
        THROW_NOT_IMPLEMENTED;
		
      out_sock->msg.clear();
      ( out_sock->msg.is_charged() 
        | socket->is_terminal_state()) . wait();
      if (socket->is_terminal_state().signalled()) {
        break;
      }
    }

    if (FD_ISSET(sock_pair[ForSelect], &rfds)) {
      break;
    }
  }
}


void OutSocket::WaitThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  socket->is_terminal_state().wait();
  static char dummy_buf[1] = {1};
  rSocketCheck(::write(notify_fd, &dummy_buf, 1) == 1);
}

}
