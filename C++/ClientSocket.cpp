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
#include "ClientSocket.h"
#include "Event.h"
#include "REvent.hpp"
#include "RState.hpp"

namespace curr {

DEFINE_AXIS(
  ClientSocketAxis,
  {  "pre_connecting", // ask_connect
      "connecting"
      },
  { 
    {"created", "closed"},
    {"created", "pre_connecting"},
    {"pre_connecting", "connecting"},
    {"connecting", "io_ready"},
    {"connecting", "connection_timed_out"},
    {"connecting", "connection_refused"},
    {"connecting", "destination_unreachable"},
    {"io_ready", "closed"}
  }
  );

DEFINE_STATES(ClientSocketAxis);

DEFINE_STATE_CONST(ClientSocket, State, created);
DEFINE_STATE_CONST(ClientSocket, State, pre_connecting);
DEFINE_STATE_CONST(ClientSocket, State, connecting);
DEFINE_STATE_CONST(ClientSocket, State, io_ready);
DEFINE_STATE_CONST(ClientSocket, State, 
                   connection_timed_out);
DEFINE_STATE_CONST(ClientSocket, State, 
                   connection_refused);
DEFINE_STATE_CONST(ClientSocket, State, 
                   destination_unreachable);
DEFINE_STATE_CONST(ClientSocket, State, closed);

ClientSocket::ClientSocket
(const ObjectCreationInfo& oi, 
 const RSocketAddress& par)
   : 
   RSocketBase(oi, par),
   RStateSplitter<ClientSocketAxis, SocketBaseAxis>
     (this, createdState,
      RStateSplitter<ClientSocketAxis, SocketBaseAxis>
      ::state_hook(&ClientSocket::state_hook)
     ),
   CONSTRUCT_EVENT(pre_connecting),
   CONSTRUCT_EVENT(connecting),
   CONSTRUCT_EVENT(io_ready),
   CONSTRUCT_EVENT(connection_timed_out),
   CONSTRUCT_EVENT(connection_refused),
   CONSTRUCT_EVENT(destination_unreachable),
   CONSTRUCT_EVENT(closed),

   thread(dynamic_cast<Thread*>
          (RSocketBase::repository->thread_factory
           -> create_thread(Thread::Par(this))))
{
   SCHECK(thread);
   RStateSplitter<ClientSocketAxis, SocketBaseAxis>
     ::init();
   this->RSocketBase::threads_terminals.push_back
      (thread->is_terminal_state());
}

ClientSocket::~ClientSocket()
{
   LOG_DEBUG(log, "~ClientSocket()");
}

void ClientSocket::ask_connect()
{
   State::move_to(*this, pre_connectingState);
   thread->start();
}

void ClientSocket::state_hook
  (AbstractObjectWithStates* object,
   const StateAxis& ax,
   const UniversalState& new_state)
{
  if (!ClientSocketAxis::is_same(ax)) {
    State::move_to(*this, 
                   RState<ClientSocketAxis>(new_state));
  }
}

void ClientSocket::process_error(int error)
{
  switch (error) {
  case EINPROGRESS:
    State::move_to(*this, connectingState);
    // <NB> there are no connecting->connecting
    // transition
    return;
  case 0:
    RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
      (*this, io_readyState);
    return;
  case ETIMEDOUT:
    RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
      (*this, connection_timed_outState);
    break;
  case ECONNREFUSED:
    RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
      (*this, connection_refusedState);
    break;
  case ENETUNREACH:
    RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
      (*this, destination_unreachableState);
    break;
  default:
    THROW_NOT_IMPLEMENTED;
  }
  //RSocketBase::process_error(error);
}

void ClientSocket::Thread::run()
{
   ThreadState::move_to(*this, workingState);
   socket->is_construction_complete_event.wait();

   auto* cli_sock = dynamic_cast<ClientSocket*>
      (socket);
   SCHECK(cli_sock);

   ( cli_sock->is_pre_connecting()
     | cli_sock->is_terminal_state()) . wait();

   if (cli_sock->is_terminal_state().signalled())
      return;

   ::connect
      (cli_sock->fd, 
       cli_sock->address->get_aw_ptr()->begin()->ai_addr, 
       cli_sock->address->get_aw_ptr()->begin()
         -> ai_addrlen);
   cli_sock->process_error(errno);

   fd_set wfds;
   FD_ZERO(&wfds);

   const SOCKET fd = socket->fd;
   SCHECK(fd >= 0);

   // Wait for termination of a connection process
   FD_SET(fd, &wfds);

   const bool use_timeout = 
     socket->repository->is_use_connect_timeout();
   const int res = ::select(
     fd+1, NULL, &wfds, NULL, 
     (use_timeout) 
     ? socket->repository->connect_timeout_timeval().get() 
     : NULL
   );
   if (res == 0) {
     // process time out by the repository request
     assert(use_timeout);
      RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
        (*cli_sock, 
         ClientSocket::connection_timed_outState);
     return;
   }
   else rSocketCheck(res > 0);
   
   LOG_DEBUG(ClientSocket::log, 
             "ClientSocket>\t ::select");

   int error = 0;
   socklen_t error_len = sizeof(error);
   rSocketCheck(
      getsockopt(fd, SOL_SOCKET, SO_ERROR, &error,
                 &error_len) == 0);

   cli_sock->process_error(error);
}

}

