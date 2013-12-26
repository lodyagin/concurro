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
    "accepted"
  },
  {
    {"created", "address_already_in_use"}, //in constructor
    {"bound", "pre_listen"},      // ask_listen()
    {"pre_listen", "listen"},
    {"listen", "accepting"}, // connect() from other side
    {"accepting", "accepted"},
    {"accepted", "listen"},
    {"listen", "closed"},
  }
  );

DEFINE_STATES(ListeningSocketAxis);

DEFINE_STATE_CONST(ListeningSocket, State, created);
DEFINE_STATE_CONST(ListeningSocket, State, bound);
DEFINE_STATE_CONST(ListeningSocket, State, 
                   address_already_in_use);
DEFINE_STATE_CONST(ListeningSocket, State, io_ready);
DEFINE_STATE_CONST(ListeningSocket, State, pre_listen);
DEFINE_STATE_CONST(ListeningSocket, State, listen);
DEFINE_STATE_CONST(ListeningSocket, State, accepting);
DEFINE_STATE_CONST(ListeningSocket, State, accepted);
DEFINE_STATE_CONST(ListeningSocket, State, closed);

ListeningSocket::ListeningSocket
  (const ObjectCreationInfo& oi, 
   const RSocketAddress& par)
 : 
  RSocketBase(oi, par),
  RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
  (this, createdState,
    RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
     ::state_hook(&ListeningSocket::state_hook)
  ),
  CONSTRUCT_EVENT(pre_listen),
  CONSTRUCT_EVENT(listen),
  CONSTRUCT_EVENT(accepted),

  select_thread(
    dynamic_cast<SelectThread*>
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
  SCHECK(select_thread);
  SCHECK(wait_thread);
  RStateSplitter<ListeningSocketAxis, SocketBaseAxis>
    ::init();
  this->RSocketBase::threads_terminals.push_back
    (select_thread->is_terminal_state());
  this->RSocketBase::threads_terminals.push_back
    (wait_thread->is_terminal_state());
}

ListeningSocket::~ListeningSocket()
{
  /*wait_and_move<ListeningSocket, ListeningSocketAxis>
    (*this, is_listen(), closedState);*/
}

void ListeningSocket::bind()
{
  assert(address);
  ::bind
    (fd, 
     address->get_aw_ptr()->begin()->ai_addr,
     address->get_aw_ptr()->begin()->ai_addrlen);
  process_bind_error(errno);
}

void ListeningSocket::ask_listen()
{
   State::move_to(*this, pre_listenState);
   select_thread->start();
   wait_thread->start();
}

void ListeningSocket::state_hook
  (AbstractObjectWithStates* object,
   const StateAxis& ax,
   const UniversalState& new_state)
{
  if (!ListeningSocketAxis::is_same(ax)) {
    const RState<ListeningSocketAxis> st(new_state);
    ListeningSocket::State::move_to(*this, st);

    if (st == io_readyState) {
      THROW_PROGRAM_ERROR; //"io_ready" state is impossible
                           // for listening socket
      // TODO add impossible states to other types of
      // sockets 
    }
  }
}

void ListeningSocket::process_bind_error(int error)
{
  switch(error) {
  case 0:
#if 1
    RSocketBase::State::move_to
      (*this, RSocketBase::boundState);
#else
    RMixedAxis<ListeningSocketAxis, SocketBaseAxis>
      ::move_to (*this, boundState);
#endif
    break;
  case EADDRINUSE:
    RMixedAxis<ListeningSocketAxis, SocketBaseAxis>
      ::move_to (*this, address_already_in_useState);
    break;
  default:
    THROW_NOT_IMPLEMENTED;
  }
}

void ListeningSocket::process_listen_error(int error)
{
  switch(error) {
  case 0:
    RAxis<ListeningSocketAxis>
      ::move_to(*this, listenState);
    break;
  default:
    THROW_NOT_IMPLEMENTED;
  }
}

void ListeningSocket::process_accept_error(int error)
{
  switch(error) {
  case 0:
    break;
  default:
    THROW_NOT_IMPLEMENTED;
  }
}

RSocketBase* ListeningSocket::get_accepted()
{
  is_accepted().wait();
  RSocketBase* sock = last_accepted;
  move_to<ListeningSocket, ListeningSocketAxis>
    (*this, listenState);
  return sock;
}

void ListeningSocket::SelectThread::run()
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

  ::listen
      (lstn_sock->fd, 
       lstn_sock->repository
       -> get_pending_connections_queue_size());
  lstn_sock->process_listen_error(errno);

  fd_set rfds;
  FD_ZERO(&rfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  for (;;) {
    FD_SET(fd, &rfds);
    FD_SET(sock_pair[ForSelect], &rfds);
    const int maxfd = std::max(sock_pair[ForSelect], fd)
      + 1;

    const int res = ::select
      (maxfd, &rfds, NULL, NULL, NULL);
    rSocketCheck(res > 0);
    LOG_DEBUG
      (ListeningSocket::log,"ListeningSocket>\t ::select");

    if (FD_ISSET(fd, &rfds)) {
      move_to<ListeningSocket, ListeningSocketAxis>
        (*lstn_sock, acceptingState);

      const SOCKET new_fd = ::accept4
        (fd, NULL, NULL, SOCK_NONBLOCK);
      lstn_sock->process_accept_error(errno);
      
      lstn_sock->last_accepted = 
        socket->repository->create_object
          (**socket->get_address()->repository
            -> create_addresses(*lstn_sock, new_fd)
              . begin());

      RAxis<ListeningSocketAxis>::move_to
        (*lstn_sock, acceptedState);
      move_to<RSocketBase, SocketBaseAxis>
        (*lstn_sock->last_accepted, 
         RSocketBase::io_readyState);

      // wait lstn_sock->get_accepted() call
      ( lstn_sock->is_listen()
      | lstn_sock->is_terminal_state() ).wait();
      if (lstn_sock->is_terminal_state().signalled())
        break;
    }
    if (FD_ISSET(sock_pair[ForSelect], &rfds)) {
      break;
    }
  }
}

void ListeningSocket::WaitThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  socket->is_terminal_state().wait();
  static char dummy_buf[1] = {1};
  rSocketCheck(::write(notify_fd, &dummy_buf, 1) == 1);
}

}
