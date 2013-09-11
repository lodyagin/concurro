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
#include "RSocketConnection.h"
#include "OutSocket.h"
#include "ClientSocket.h"
#include "REvent.hpp"
#include "RState.hpp"
#include "RWindow.hpp"

namespace curr {

DEFINE_AXIS(
  ClientConnectionAxis,
  { "aborting", // skiping data and closing buffers
     "aborted",   // after aborting
     "clearly_closed" // all pending data 
                      // was received / sent
  },
  { { "ready", "aborting" },
    { "aborting", "aborted" },
    { "closed", "clearly_closed" },
    { "closed", "aborting" }
  }
);

DEFINE_STATES(ClientConnectionAxis);

DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   aborting);
DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   aborted);
DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   clearly_closed);

std::ostream&
operator<< (std::ostream& out, const RSocketConnection& c)
{
  out << "<connection>";
  return out;
}

RSocketConnection::RSocketConnection
(const ObjectCreationInfo& oi,
 const Par& par)
  : StdIdMember(oi.objectId),
    socket_rep(std::move(par.socket_rep))
{
  assert(socket_rep);
}

RSingleSocketConnection::RSingleSocketConnection
(const ObjectCreationInfo& oi,
 const Par& par)
  : RSocketConnection(oi, par),
    RStateSplitter
    (dynamic_cast<ClientSocket*>(par.socket), 
     ClientSocket::createdState),
    CONSTRUCT_EVENT(aborting),
    CONSTRUCT_EVENT(aborted),
    CONSTRUCT_EVENT(clearly_closed),
    socket(dynamic_cast<InSocket*>(par.socket)),
    cli_sock(dynamic_cast<ClientSocket*>(par.socket)),
    thread(dynamic_cast<SocketThread*>
           (RThreadRepository<RThread<std::thread>>
            ::instance().create_thread
            (*par.get_thread_par(this)))),
    in_win(RConnectedWindowRepository<SOCKET>::instance()
           .create_object(*par.get_window_par(cli_sock))),
    is_closed_event(cli_sock, "closed"),
    is_terminal_state_event { 
      is_clearly_closed_event,
      is_aborted_event,
      socket->is_terminal_state()
    }
{
  assert(socket);
  assert(cli_sock);
  SCHECK(thread);
  SCHECK(in_win);
  RStateSplitter::init();
  thread->start();
}

RSingleSocketConnection::~RSingleSocketConnection()
{
  //TODO not only TCP
  //dynamic_cast<TCPSocket*>(socket)->ask_close();
  //socket->ask_close_out();
  is_terminal_state_event.wait();
  socket_rep->delete_object(socket, true);
}

void RSingleSocketConnection::state_changed
  (StateAxis&, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState&
   )
{
  //FIXME no parent call

  RState<ClientConnectionAxis> st =
    state_ax.bound(object->current_state(state_ax));

  // aborting state check
  if (st == RState<ClientConnectionAxis>
      (ClientSocket::closedState)
      && RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
      ::compare_and_move
      (*this, abortingState, abortedState))
    return;

  // aborted state check
  if (st == RState<ClientConnectionAxis>
      (ClientSocket::closedState)
      && A_STATE(RSingleSocketConnection, 
                 ClientConnectionAxis, state_is, aborted))
    return;

  RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
    ::neg_compare_and_move(*this, st, st);

  LOG_TRACE(log, "moved to " << st);
}

RSocketConnection& RSingleSocketConnection
::operator<< (const std::string& str)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  (out_sock->msg.is_discharged() | out_sock->msg.is_dummy()).wait();
  out_sock->msg.reserve(str.size(), 0);
  ::strncpy((char*)out_sock->msg.data(), str.c_str(),
            str.size());
  out_sock->msg.resize(str.size());
  return *this;
}

RSocketConnection& RSingleSocketConnection
::operator<< (RSingleBuffer&& buf)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  (out_sock->msg.is_discharged() | out_sock->msg.is_dummy()).wait();
  out_sock->msg.move(&buf);
  return *this;
}

void RSingleSocketConnection::ask_connect()
{
  dynamic_cast<ClientSocket*>(socket)->ask_connect();
}

void RSingleSocketConnection::ask_close()
{
  socket->ask_close_out();
}

void RSingleSocketConnection::run()
{
  //<NB> it is not a thread run(), it is called from it
  socket->is_construction_complete_event.wait();

  for (;;) {
    // The socket has a message.
    ( socket->msg.is_charged()
      | is_aborting() | is_terminal_state() ). wait();

    // We already need it.
    ( iw().is_wait_for_buffer()
      | is_aborting() | is_terminal_state() ). wait();

    if (is_aborting().signalled()) 
      goto LAborting;
    else if (is_terminal_state().signalled()) 
      goto LClosed;

    std::unique_ptr<RSingleBuffer> buf 
      (new RSingleBuffer(&socket->msg));

    // a content of the buffer will be cleared after
    // everybody stops using it
    buf->set_autoclear(true);
    iw().new_buffer(std::move(buf));
  }

LAborting:
  is_aborting().wait();
  ask_close();
  socket->InSocket::is_terminal_state().wait();
  // No sence to start aborting while a socket is working
  iw().detach();

LClosed:
  if (!STATE_OBJ(RBuffer, state_is, socket->msg, 
                 discharged))
    socket->msg.clear();
}

void RSingleSocketConnection::ask_abort()
{
#if 1
  A_STATE(RSingleSocketConnection, 
          ClientConnectionAxis, move_to, aborting);
#else
  // we block because no connecting -> aborting transition
  // (need change ClientSocket to allow it)
  is_can_abort().wait();

  if (RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
      ::compare_and_move
      (*this, ClientSocket::connectedState,
       RSingleSocketConnection::abortingState))
    is_aborted().wait();
#endif
}
}
