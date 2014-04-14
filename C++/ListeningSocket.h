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

#ifndef CONCURRO_LISTENINGSOCKET_H_
#define CONCURRO_LISTENINGSOCKET_H_

#include "RSocket.h"
#include "RMutex.h"
#include <stack>

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

DECLARE_AXIS(ListeningSocketAxis, SocketBaseAxis);

/**
  * Just a marker class for a socket which use accept()
  * system call for generating new ServerSocket -s. Really
  * all its functionality is implemented in TCPSocket.
  *
  * @dot
  * digraph {
  *   start [shape = point]; 
  *   address_already_in_use [shape = doublecircle];
  *   closed [shape = doublecircle];
  *   start -> created;
  *   created -> bound;
  *   created -> address_already_in_use;
  *   bound -> pre_listen;
  *   pre_listen -> listen;
  *   listen -> accepting;
  *   accepting -> accepted;
  *   accepted -> listen;
  *   listen -> closed;
  *   bound -> closed;
  *   closed -> closed;
  * }
  * @enddot
  */
class ListeningSocket 
  : virtual public RSocketBase,
    public RStateSplitter
      <ListeningSocketAxis, SocketBaseAxis, 1, 1>
{
  DECLARE_EVENT(ListeningSocketAxis, pre_listen);
  DECLARE_EVENT(ListeningSocketAxis, listen);
  DECLARE_EVENT(ListeningSocketAxis, accepted);

public:
  using ParentSplitter = RStateSplitter
    <ListeningSocketAxis, SocketBaseAxis, 1, 1>;

  //! @cond
  DECLARE_STATES(ListeningSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, bound);
  DECLARE_STATE_CONST(State, address_already_in_use);
  DECLARE_STATE_CONST(State, io_ready);
  DECLARE_STATE_CONST(State, pre_listen);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, accepted);
  DECLARE_STATE_CONST(State, closed);
  //! @endcond

  ~ListeningSocket();

  void bind() override;

  //! Start listening for incoming connections. It moves
  //! the object in the `listen' state.
  virtual void ask_listen();

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
    return ParentSplitter::current_state(ax);
  }

  const std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) const override
  { 
    return ParentSplitter::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return ParentSplitter::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }

  //! Wait the accepted state, return the accepted socket
  //! and move to listen state.
  RSocketBase* get_accepted();

protected:
  //const CompoundEvent is_terminal_state_event;

  ListeningSocket
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
        thread_name = SFORMAT("ListeningSocket(select):" 
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
          thread_name = SFORMAT("ListeningSocket(wait):" 
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

  RSocketBase* last_accepted = nullptr;

  void process_bind_error(int error);
  void process_listen_error(int error);
  void process_accept_error(int error);

  DEFAULT_LOGGER(ListeningSocket);
};


//! @}

}
#endif
