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

#include "ListeningSocket.h"
#include "RState.hpp"
#include "REvent.hpp"

namespace curr {

DEFINE_AXIS(
  ListeningSocketAxis, 
  {
    "pre_listen",
    "listen",       // passive open
    "accepting",    // in the middle of a new ServerSocket
  },
  {
    {"created", "pre_listen"},      // ask_listen()
    {"pre_listen", "listen"},
    {"listen", "accepting"}, // connect() from other side
    {"accepting", "listen"},
    {"listen", "closed"},
  }
  );

DEFINE_STATES(ListeningSocketAxis);

DEFINE_STATE_CONST(ListeningSocket, State, created);
DEFINE_STATE_CONST(ListeningSocket, State, ready);
DEFINE_STATE_CONST(ListeningSocket, State, pre_listen);
DEFINE_STATE_CONST(ListeningSocket, State, listen);
DEFINE_STATE_CONST(ListeningSocket, State, accepting);
DEFINE_STATE_CONST(ListeningSocket, State, closed);

ListeningSocket::ListeningSocket
  (const ObjectCreationInfo& oi, 
   const RSocketAddress& par)
 : 
  RSocketBase(oi, par),
  RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
    (this, createdState/*,
    RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
     ::state_hook(&ServerSocket::state_hook)*/
     ),
  CONSTRUCT_EVENT(pre_listen),

  thread(dynamic_cast<Thread*>
       (RSocketBase::repository->thread_factory
        -> create_thread(Thread::Par(this))))
{
  SCHECK(thread);
  RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
    ::init();
  this->RSocketBase::threads_terminals.push_back
    (thread->is_terminal_state());

  ::bind
    (fd, 
     par.get_aw_ptr()->begin()->ai_addr,
     par.get_aw_ptr()->begin()->ai_addrlen);
  process_error(errno);
}

void ListeningSocket::ask_listen()
{
   State::move_to(*this, pre_listenState);
   thread->start();
}

void ListeningSocket::state_hook
  (AbstractObjectWithStates* object,
   const StateAxis& ax,
   const UniversalState& new_state)
{
  if (!ListeningSocketAxis::is_same(ax)) {
    const RState<ListeningSocketAxis> st(new_state);
    ListeningSocket::State::move_to(*this, st);

    if (st == readyState) {
      THROW_PROGRAM_ERROR; // "ready" state is impossible
                           // for listening socket
    }
  }
}

void ListeningSocket::process_error(int error)
{
  switch(error) {
  case 0:
    return;
  default:
    THROW_NOT_IMPLEMENTED;
  }
}

void ListeningSocket::Thread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  auto* lstn_sock = dynamic_cast<ListeningSocket*>
    (socket);
  SCHECK(lstn_sock);

  ( lstn_sock->is_pre_listen()
    | lstn_sock->is_terminal_state()) . wait();

  if (lstn_sock->is_terminal_state().signalled())
    return;
#if 0
  ::listen
      (lstn_sock->fd, 
       lstn_sock->repository
       -> get_pending_connections_queue_size());

  lstn_sock->process_error(errno);
#endif
  fd_set rfds;
  FD_ZERO(&rfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  FD_SET(fd, &rfds);

  const int res = ::select(fd+1, &rfds, NULL, NULL, NULL);
  rSocketCheck(res > 0);

  LOG_DEBUG
    (ListeningSocket::log, "ListeningSocket>\t ::select");

  int error = 0;
  socklen_t error_len = sizeof(error);
  rSocketCheck(
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error,
               &error_len) == 0);

  lstn_sock->process_error(error);
}

}
