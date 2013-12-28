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

#ifndef CONCURRO_RSOCKETCONNECTION_HPP_
#define CONCURRO_RSOCKETCONNECTION_HPP_

#include "RSocketConnection.h"
#include "RSocketAddress.hpp"
#include "RObjectWithThreads.hpp"
#include "OutSocket.h"
#include "ClientSocket.h"

namespace curr {

namespace connection {

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
int basic_streambuf<CharT, Traits>
//
::sync()
{
  // send the current buffer
  c->out_buf.resize(this->pptr() - this->pbase());
  if (!c->push_out())
    return -1;

  // reinitialize the buffer
  c->out_buf.reserve(c->max_packet_size, 0);

  // start charging
  CharT* ptr = static_cast<CharT*>(c->out_buf.data());
  this->setp
    (ptr, 
     ptr, 
     ptr + c->out_buf.capacity() / sizeof(CharT));

  return 0;
}

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
int_type basic_streambuf<CharT, Traits>
//
::underflow()
{
  // release the current input buffer
  c->in_buf.clear();

  // get new data
  if (!c->pull_in())
    return traits::eof();

  // start reading
  CharT* ptr = const_cast<CharT*>
    (reinterpret_cast<const CharT*>(in.cdata()));
  this->setg
    (ptr, 
     ptr, 
     ptr + in.filled_size() / size(CharT));

  return Traits::to_int_type(*ptr);
}

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
int_type basic_streambuf<CharT, Traits>
//
::overflow(int_type ch)
{
  if (sync() == 0) {
    if (!Traits::eq_int_type(ch, Traits::eof())) {
      assert(epptr() > pbase());
      *pbase() = Traits::to_char_type(ch);
      pbump(1);
    }
    return Traits::eof() + 1;
  }
  return Traits::eof();
}

DEFINE_AXIS_TEMPL(
  SocketConnectionAxis, RSocketBase,
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

namespace single_socket {

#define CURR_RSOCKETCONNECTION_TEMPL_ template \
< \
  class Connection, \
  class Socket, \
  class... Threads \
>
#define CURR_RSOCKETCONNECTION_T_ \
  Connection, Socket, Threads...

CURR_RSOCKETCONNECTION_TEMPL_
const RState
  <typename connection
    <CURR_RSOCKETCONNECTION_T_>::State::axis>
connection<CURR_RSOCKETCONNECTION_T_>
::abortingState("aborting");

CURR_RSOCKETCONNECTION_TEMPL_
const RState
  <typename connection
    <CURR_RSOCKETCONNECTION_T_>::State::axis>
connection<CURR_RSOCKETCONNECTION_T_>
::abortedState("aborted");

CURR_RSOCKETCONNECTION_TEMPL_
const RState
  <typename connection
    <CURR_RSOCKETCONNECTION_T_>::State::axis>
connection<CURR_RSOCKETCONNECTION_T_>
::clearly_closedState("clearly_closed");

#if 0
CURR_RSOCKETCONNECTION_TEMPL_
template<NetworkProtocol proto, IPVer ip_ver>
abstract_connection* 
connection<CURR_RSOCKETCONNECTION_T_>
::InetClientPar<proto, ip_ver>
//
::create_derivation(const ObjectCreationInfo& oi) const
{
  assert(this->sock_addr);
  this->socket_rep = //FIXME memory leak
    new RSocketRepository(
  SFORMAT(typeid(Connection).name() 
          << ":" << oi.objectId
          << ":RSocketRepository"),
          max_input_packet,
          dynamic_cast<RConnectionRepository*>
            (oi.repository)->thread_factory
          );
  this->socket_rep->set_connect_timeout_u
    (max_connection_timeout);
  this->socket = this->socket_rep->create_object
    (*this->sock_addr);
  return new Connection(oi, *this);
}
#endif


CURR_RSOCKETCONNECTION_TEMPL_
connection<CURR_RSOCKETCONNECTION_T_>
//
::connection
  (const ObjectCreationInfo& oi,
   const Par& par)
  : 
    abstract_connection(oi, par),
    Splitter
      (dynamic_cast<Socket*>(par.socket), 
       RState<typename Socket::State::axis>
         (dynamic_cast<Socket&>(*par.socket))),
    CONSTRUCT_EVENT(aborting),
    CONSTRUCT_EVENT(aborted),
    CONSTRUCT_EVENT(clearly_closed),
    CONSTRUCT_EVENT(io_ready),
    CONSTRUCT_EVENT(closed),
    socket_rep(par.socket_rep),
    in_sock(dynamic_cast<InSocket*>(par.socket)),
    out_sock(dynamic_cast<OutSocket*>(par.socket)),
    is_terminal_state_event { 
      is_clearly_closed_event,
      is_aborted_event,
      in_sock->is_terminal_state()
    },
    out_buf(max_packet_size, 0)
{
  assert(socket_rep);
  assert(socket); // FIXME can be nullptr (output only)

  Splitter::init();
  //SCHECK(RState<ConnectedWindowAxis>(*in_win) == 
  //       RConnectedWindow::readyState);
}

CURR_RSOCKETCONNECTION_TEMPL_
connection<CURR_RSOCKETCONNECTION_T_>
//
::~connection()
{
  SCHECK(this->destructor_delegate_is_called);
  is_terminal_state_event.wait();
  socket_rep->delete_object(socket, true);
}

CURR_RSOCKETCONNECTION_TEMPL_
void connection<CURR_RSOCKETCONNECTION_T_>
//
::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object,
   const UniversalState& new_state
   )
{
  RObjectWithThreads<Connection>::state_changed
    (ax, state_ax, object, new_state);

  if (!SocketConnectionAxis<Socket>::is_same(ax))
    return;

  RState<SocketConnectionAxis<Socket>> st =
    state_ax.bound(object->current_state(state_ax));

  // aborting state check
  if (st == RState<SocketConnectionAxis<Socket>>
        (ClientSocket::closedState)
      && compare_and_move
         <
           connection
             <CURR_RSOCKETCONNECTION_T_>, 
           SocketConnectionAxis<Socket>
         >
         (*this, abortingState, abortedState)
      )
    return;

  // aborted state check
  if (st == RState<SocketConnectionAxis<Socket>>
      (ClientSocket::closedState)
      && A_STATE(connection, 
                 SocketConnectionAxis<Socket>, 
                 state_is, aborted))
    return;

  neg_compare_and_move
  <
     connection
       <CURR_RSOCKETCONNECTION_T_>, 
     SocketConnectionAxis<Socket>
  >
  (*this, st, st);

  LOG_TRACE(log, "moved to " << st);
}

#if 0
template<class Connection, class Socket, class... Threads>
RSocketConnection& 
connection<CURR_RSOCKETCONNECTION_T_>
//
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

template<class Connection, class Socket, class... Threads>
RSocketConnection& connection
  <CURR_RSOCKETCONNECTION_T_>
//
::operator<< (RSingleBuffer&& buf)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  ( out_sock->msg.is_discharged() 
  | out_sock->msg.is_dummy() ).wait();
  out_sock->msg.move(&buf);
  return *this;
}
#endif

CURR_RSOCKETCONNECTION_TEMPL_
void connection
  <Connection, Socket,Threads...>
//
::ask_connect()
{
  auto* cs = dynamic_cast<ClientSocket*>(socket);
  SCHECK(cs); // not a client side of a connection
  cs->ask_connect();
}

CURR_RSOCKETCONNECTION_TEMPL_
void connection<CURR_RSOCKETCONNECTION_T_>
//
::ask_close()
{
  if (out_sock)
    out_sock->ask_close_out();
}

CURR_RSOCKETCONNECTION_TEMPL_
void connection<CURR_RSOCKETCONNECTION_T_>
//
::ask_abort()
{
#if 1
  A_STATE(connection, 
          SocketConnectionAxis<Socket>, move_to, aborting);
#else
  // we block because no connecting -> aborting transition
  // (need change ClientSocket to allow it)
  is_can_abort().wait();

  if (RMixedAxis<SocketConnectionAxis<Socket>, ClientSocketAxis>
      ::compare_and_move
      (*this, ClientSocket::connectedState,
       connection::abortingState))
    is_aborted().wait();
#endif
}

CURR_RSOCKETCONNECTION_TEMPL_
bool connection<CURR_RSOCKETCONNECTION_T_>
//
::push_out()
{
  SCHECK(out_sock);

  CompoundEvent out_ready = 
    ( out_sock->msg.is_discharged() 
    | out_sock->msg.is_dummy());

  ( out_ready
  | this->is_aborting() 
  | this->is_terminal_state() ). wait();

  if (out_ready.signalled()) {
    out_sock->msg.move(&this->out_buf);
    return true;
  }
  else return false;
}

CURR_RSOCKETCONNECTION_TEMPL_
bool connection<CURR_RSOCKETCONNECTION_T_>
//
::pull_in()
{
  SCHECK(in_sock);

  // wait a new IP packet
  ( in_sock->msg.is_charged()
  | this->is_aborting() 
  | this->is_terminal_state() ). wait();

  LOG_DEBUG
    (clog, *in_sock << " has a new state or packet");

  if (in_sock->msg.is_charged().signalled()) {
    in_buf.move(&in_sock->msg);
    return true;
  }
  else return false;
}

template<
  class Parent,
  class Connection, 
  class Socket, 
  class... Threads
>
bulk<Parent, Connection, Socket, Threads...>
//
::bulk(const ObjectCreationInfo& oi, const Par& par)
  :
  Parent(oi, par),
  in_win(RConnectedWindowRepository<SOCKET>::instance()
    .create_object(*par.get_window_par(in_sock)))
{
  SCHECK(in_win);
}

template<
  class Parent,
  class Connection, 
  class Socket, 
  class... Threads
>
void bulk<Parent, Connection, Socket, Threads...>
//
::run()
{
  typedef Logger<LOG::Connections> clog;

  //<NB> it is not a thread run(), it is called from it
  in_sock->is_construction_complete_event.wait();

  for (;;) {
    if (pull_in()) {
      iw().is_wait_for_buffer().wait();
      LOG_DEBUG(clog, iw() << " asked for more data");
      iw().new_buffer(std::move(in_buf));
    }
    else {
      if (is_aborting().signalled()) {
        LOG_DEBUG(clog, "the connection is aborting");
        goto LAborting;
      }
      else if (is_terminal_state().signalled()) {
        LOG_DEBUG(clog, "the connection is closing");
        goto LClosed;
      }
      SCHECK(false);
    }
  }

LAborting:
  is_aborting().wait();
  ask_close();
  in_sock->InSocket::is_terminal_state().wait();
  // No sence to start aborting while a socket is working
  iw().detach();

LClosed:
  if (!STATE_OBJ(RBuffer, state_is, in_sock->msg, 
                 discharged))
    in_sock->msg.clear();
}

} // namespace single_socket
} // namespace connection

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

