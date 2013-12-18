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

#ifndef CONCURRO_RSOCKETCONNECTION_HPP_
#define CONCURRO_RSOCKETCONNECTION_HPP_

#include "RSocketConnection.h"
#include "RSocketAddress.hpp"
#include "RObjectWithThreads.hpp"
#include "OutSocket.h"
#include "ClientSocket.h"

namespace curr {

DEFINE_AXIS_TEMPL(
  Socket,
  SocketConnectionAxis,
  { "aborting", // skipping data and closing buffers
     "aborted",   // after aborting
     "clearly_closed" // all pending data 
                      // was received / sent
  },
  { { "io_ready", "aborting" },
    { "aborting", "aborted" },
    { "closed", "clearly_closed" },
    { "closed", "aborting" }
  }
);

template<class Socket>
DEFINE_STATE_CONST(RSingleSocketConnection<Socket>, State, 
                   aborting);

template<class Socket>
DEFINE_STATE_CONST(RSingleSocketConnection<Socket>, State, 
                   aborted);

template<class Socket>
DEFINE_STATE_CONST(RSingleSocketConnection<Socket>, State, 
                   clearly_closed);

template<class Socket>
RSingleSocketConnection<Socket>::RSingleSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
  : 
    RSocketConnection(oi, par),
    Splitter
      (dynamic_cast<Socket*>(par.socket), 
       Socket::createdState),
    CONSTRUCT_EVENT(aborting),
    CONSTRUCT_EVENT(aborted),
    CONSTRUCT_EVENT(clearly_closed),
    CONSTRUCT_EVENT(io_ready),
    CONSTRUCT_EVENT(closed),
    socket(dynamic_cast<InSocket*>(par.socket)),

    //cli_sock(dynamic_cast<ClientSocket*>(par.socket)),
    thread(dynamic_cast<SocketThread*>
           (StdThreadRepository::instance().create_thread
            (*par.get_thread_par(this)))),
    in_win(RConnectedWindowRepository<SOCKET>::instance()
           .create_object(*par.get_window_par(socket))),
    //is_closed_event(socket, "closed"),
    is_terminal_state_event { 
      is_clearly_closed_event,
      is_aborted_event,
      socket->is_terminal_state()
    }
{
  assert(socket);
//  assert(cli_sock);
  SCHECK(thread);
  SCHECK(in_win);
  Splitter::init();
  thread->start();
}

template<class Socket>
RSingleSocketConnection<Socket>::~RSingleSocketConnection()
{
  //TODO not only TCP
  //dynamic_cast<TCPSocket*>(socket)->ask_close();
  //socket->ask_close_out();
  is_terminal_state_event.wait();
  socket_rep->delete_object(socket, true);
}

template<class Socket>
void RSingleSocketConnection<Socket>::state_changed
  (StateAxis&, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState&
   )
{
  //FIXME no parent call

  RState<SocketConnectionAxis<Socket>> st =
    state_ax.bound(object->current_state(state_ax));

  // aborting state check
  if (st == RState<SocketConnectionAxis<Socket>>
        (ClientSocket::closedState)
      && compare_and_move
         <
          RSingleSocketConnection<Socket>, 
          SocketConnectionAxis<Socket>
         >
         (*this, abortingState, abortedState)
      )
    return;

  // aborted state check
  if (st == RState<SocketConnectionAxis<Socket>>
      (ClientSocket::closedState)
      && A_STATE(RSingleSocketConnection, 
                 SocketConnectionAxis<Socket>, 
                 state_is, aborted))
    return;

  neg_compare_and_move
  <
     RSingleSocketConnection<Socket>, 
     SocketConnectionAxis<Socket>
  >
  (*this, st, st);

  LOG_TRACE(log, "moved to " << st);
}

template<class Socket>
RSocketConnection& RSingleSocketConnection<Socket>
::operator<< (const std::string& str)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  ( out_sock->msg.is_discharged() 
  | out_sock->msg.is_dummy() ).wait();
  out_sock->msg.reserve(str.size(), 0);
  ::strncpy((char*)out_sock->msg.data(), str.c_str(),
            str.size());
  out_sock->msg.resize(str.size());
  return *this;
}

template<class Socket>
RSocketConnection& RSingleSocketConnection<Socket>
::operator<< (RSingleBuffer&& buf)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  ( out_sock->msg.is_discharged() 
  | out_sock->msg.is_dummy() ).wait();
  out_sock->msg.move(&buf);
  return *this;
}

template<class Socket>
void RSingleSocketConnection<Socket>::ask_connect()
{
  dynamic_cast<ClientSocket*>(socket)->ask_connect();
}

template<class Socket>
void RSingleSocketConnection<Socket>::ask_close()
{
  socket->ask_close_out();
}

template<class Socket>
void RSingleSocketConnection<Socket>::run()
{
  typedef Logger<LOG::Connections> clog;

  //<NB> it is not a thread run(), it is called from it
  socket->is_construction_complete_event.wait();

  for (;;) {
    // The socket has a message.
    ( socket->msg.is_charged()
      | is_aborting() | is_terminal_state() ). wait();

    LOG_DEBUG(clog, 
              *socket << " has a new state or packet");

    // We already need it.
    ( iw().is_wait_for_buffer()
      | is_aborting() | is_terminal_state() ). wait();

    if (is_aborting().signalled()) {
      LOG_DEBUG(clog, "the connection is aborting");
      goto LAborting;
    }
    else if (is_terminal_state().signalled()) {
      LOG_DEBUG(clog, "the connection is closing");
      goto LClosed;
    }

    LOG_DEBUG(clog, iw() << " asked for more data");

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

template<class Socket>
void RSingleSocketConnection<Socket>::ask_abort()
{
#if 1
  A_STATE(RSingleSocketConnection, 
          SocketConnectionAxis<Socket>, move_to, aborting);
#else
  // we block because no connecting -> aborting transition
  // (need change ClientSocket to allow it)
  is_can_abort().wait();

  if (RMixedAxis<SocketConnectionAxis<Socket>, ClientSocketAxis>
      ::compare_and_move
      (*this, ClientSocket::connectedState,
       RSingleSocketConnection::abortingState))
    is_aborted().wait();
#endif
}

template<class Connection>
RServerConnectionFactory<Connection>
//
::RServerConnectionFactory
  (ListeningSocket* l_sock, size_t reserved)
: 
  RStateSplitter
    <ServerConnectionFactoryAxis, ListeningSocketAxis>
      (l_sock, ListeningSocket::boundState),
  RConnectionRepository
    ( typeid(*this).name(), 
      reserved,
      &StdThreadRepository::instance()),
  threads(this),
  lstn_sock(l_sock)
{
  assert(lstn_sock);
  const bool isBound = state_is
    <ListeningSocket, ListeningSocketAxis>
      (*lstn_sock, RSocketBase::boundState);
  SCHECK(isBound);
  threads.complete_construction();
}

template<class Connection>
RServerConnectionFactory<Connection>::Threads
//
::Threads(RServerConnectionFactory* o) :
  RObjectWithThreads<Threads>
  { 
    new typename RServerConnectionFactory<Connection>
      ::ListenThread::Par() 
  },
  obj(o)
{
}

template<class Connection>
void RServerConnectionFactory<Connection>
//
::ListenThread::run()
{
  RServerConnectionFactory* fact = this->object->obj;
  ListeningSocket* lstn_sock = fact->lstn_sock;

  assert(lstn_sock);
  move_to(*this, RThreadBase::workingState);

  lstn_sock->ask_listen();
  for (;;) {

    ( lstn_sock->is_accepted() 
    | this->isStopRequested
    ).wait();

    if (this->isStopRequested.signalled())
      break;

    RSocketBase* sock = lstn_sock->get_accepted();
    this->object->obj->create_object
      (typename Connection::ServerPar(sock));
  }
}

}

#endif

