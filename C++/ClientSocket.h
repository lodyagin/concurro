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

#ifndef CONCURRO_CLIENTSOCKET_H_
#define CONCURRO_CLIENTSOCKET_H_

#include "RSocket.h"

namespace curr {

DECLARE_AXIS(ClientSocketAxis, SocketBaseAxis);

/**
 * A socket component which responds to performing
 * aclient-side connection.
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   connection_timed_out [shape = doublecircle];
 *   connection_refused [shape = doublecircle];
 *   destination_unreachable [shape = doublecircle];
 *   closed [shape = doublecircle];
 *   start -> created;
 *   created -> io_ready;
 *   created -> connection_timed_out;
 *   created -> connection_refused;
 *   created -> destination_unreachable;
 *   io_ready -> closed;
 *   closed -> closed;
 *   created -> closed;
 *   created -> pre_connecting;
 *   pre_connecting -> connecting;
 *   connecting -> io_ready;
 *   connecting -> connection_timed_out;
 *   connecting -> connection_refused;
 *   connecting -> destination_unreachable;
 *   io_ready -> closed;
 * }
 * @enddot
 *
 */
class ClientSocket : virtual public RSocketBase,
  public RStateSplitter<ClientSocketAxis, SocketBaseAxis>
{
  DECLARE_EVENT(ClientSocketAxis, pre_connecting);
  DECLARE_EVENT(ClientSocketAxis, connecting);
  DECLARE_EVENT(ClientSocketAxis, io_ready);
  DECLARE_EVENT(ClientSocketAxis, connection_timed_out);
  DECLARE_EVENT(ClientSocketAxis, connection_refused);
  DECLARE_EVENT(ClientSocketAxis, destination_unreachable);
  DECLARE_EVENT(ClientSocketAxis, closed);

public:
  //! @cond
  DECLARE_STATES(ClientSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, pre_connecting);
  DECLARE_STATE_CONST(State, connecting);
  DECLARE_STATE_CONST(State, io_ready);
  DECLARE_STATE_CONST(State, connection_timed_out);
  DECLARE_STATE_CONST(State, connection_refused);
  DECLARE_STATE_CONST(State, destination_unreachable);
  DECLARE_STATE_CONST(State, closed);
  //! @endcond

  const CompoundEvent is_terminal_state_event;

  /*CompoundEvent is_terminal_state() const override
    {
    return is_terminal_state_event;
    }*/

  std::string object_name() const override
  {
    return SFORMAT("ClientSocket:" << universal_id());
  }

  ~ClientSocket();

  //! Start connection to a server. It can result in
  //! connecting -> {io_ready, connection_timed_out,
  //! connection_refused and destination_unreachable}
  //! states.
  virtual void ask_connect();

  std::string universal_id() const override
  {
    return RSocketBase::universal_id();
  }

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    THROW_PROGRAM_ERROR;
  }

  void state_hook
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  { 
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }

protected:
  ClientSocket
    (const ObjectCreationInfo& oi, 
     const RSocketAddress& par);

  class Thread : public SocketThread
  {
  public:
    struct Par : public SocketThread::Par
    { 
    Par(RSocketBase* sock) 
      : SocketThread::Par(sock) 
      {
        thread_name = SFORMAT
          ("ClientSocket(connect):" << sock->fd);
      }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new Thread(oi, *this); 
      }
    };

    void run();
  protected:
    Thread(const ObjectCreationInfo& oi, const Par& p)
      : SocketThread(oi, p) {}
    ~Thread() { destroy(); }
  }* thread;

  void process_error(int error);

  DEFAULT_LOGGER(ClientSocket);
};

}
#endif
