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

#ifndef CONCURRO_TCPSOCKET_H_
#define CONCURRO_TCPSOCKET_H_

#include <netdb.h>
#include "Logging.h"
#include "RSocket.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "ListeningSocket.h"

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

DECLARE_AXIS(TCPAxis, ListeningSocketAxis);

/**
  * This socket component manages TCP proto states.
  *
  * @dot
  * digraph {
  *   start [shape = point]; 
  *   closed [shape = doublecircle];
  *   start -> created;
  *   created -> listen;
  *   listen -> accepting;
  *   accepting -> listen;
  *   listen -> closed;
  *   created -> io_ready;
  *   created -> closed;
  *   io_ready -> closing;
  *   io_ready -> in_closed;
  *   io_ready -> out_closed;
  *   closing -> closed;
  *   in_closed -> closed;
  *   out_closed -> closed;
  *   closed -> closed;
  * }
  * @enddot
  */
class TCPSocket : virtual public RSocketBase
, public RStateSplitter<TCPAxis, SocketBaseAxis>
{
  DECLARE_EVENT(TCPAxis, io_ready);
  DECLARE_EVENT(TCPAxis, closed);
  DECLARE_EVENT(TCPAxis, in_closed);
  DECLARE_EVENT(TCPAxis, out_closed);

public:
  DECLARE_STATES(TCPAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, in_closed);
  DECLARE_STATE_CONST(State, out_closed);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, io_ready);
  DECLARE_STATE_CONST(State, closing);

  ~TCPSocket();

  //! Ask to close an outbound part
  void ask_close_out() override;

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
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::current_state(ax);
  }

#if 0
  Event get_event (const UniversalEvent& ue) override
  {
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::get_event(ue);
  }

  Event get_event (const UniversalEvent& ue) const override
  {
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::get_event(ue);
  }
#endif

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
#if 1
    ax.update_events(this, trans_id, to);
#else
    LOG_TRACE(log, "update_events");
    return RStateSplitter<TCPAxis,SocketBaseAxis>
      ::update_events(ax, trans_id, to);
#endif
}

protected:
  //! It is set in the constructor by 
  //! ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  //! Create a TCP socket in a "closed" state.
  TCPSocket
    (const ObjectCreationInfo& oi, 
     const RSocketAddress& par);

  class SelectThread : public SocketThreadWithPair
  {
  public:
    struct Par : public SocketThreadWithPair::Par
    {
    Par(RSocketBase* sock) 
      : SocketThreadWithPair::Par(sock)
      {
        thread_name = SFORMAT("TCPSocket(select):" 
                              << sock->fd);
      }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new SelectThread(oi, *this); 
      }
    };

  protected:
    SelectThread
      (const ObjectCreationInfo& oi, const Par& p)
      : SocketThreadWithPair(oi, p) {}
    ~SelectThread() { destroy(); }
    void run();
  }* select_thread;

  class WaitThread : public SocketThread
  {
  public:
    struct Par : public SocketThread::Par
    { 
      SOCKET notify_fd;
      Par(RSocketBase* sock, SOCKET notify) 
        : SocketThread::Par(sock),
        notify_fd(notify)
        {
          thread_name = SFORMAT("TCPSocket(wait):" 
                                << sock->fd);
        }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new WaitThread(oi, *this); 
      }
    };

    void run();

  protected:
    SOCKET notify_fd;

    WaitThread
      (const ObjectCreationInfo& oi, const Par& p)
      : SocketThread(oi, p), notify_fd(p.notify_fd) {}
    ~WaitThread() { destroy(); }
  }* wait_thread;

private:
  typedef Logger<TCPSocket> log;
};

//! @}

}
#endif

