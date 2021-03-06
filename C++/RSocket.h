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

#ifndef CONCURRO_RSOCKET_H_
#define CONCURRO_RSOCKET_H_

#include "SNotCopyable.h"
#include "REvent.h"
#include "Repository.h"
#include "RBuffer.h"
#include "RSocketAddress.h"
#include "RThread.h"
#include "RState.h"
#include "RThreadRepository.h"
#ifndef _WIN32
#  define SOCKET int
#endif
#include <list>

namespace curr {

/**
 * @defgroup sockets
 * Just sockets.
 * @{
 */

DECLARE_AXIS(SocketBaseAxis, StateAxis);

class RSocketRepository;
class SocketThread;

/**
 * An abstract socket base.
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   connection_timed_out [shape = doublecircle];
 *   connection_refused [shape = doublecircle];
 *   destination_unreachable [shape = doublecircle];
 *   address_already_in_use [shape = doublecircle];
 *   closed [shape = doublecircle];
 *   start    -> created;
 *   created  -> io_ready;
 *   created  -> bound;
 *   created  -> connection_timed_out;
 *   created  -> connection_refused;
 *   created  -> destination_unreachable;
 *   created  -> address_already_in_use;
 *   created  -> closed;
 *   bound    -> closed;
 *   io_ready -> closed;
 *   closed   -> closed;
 * }
 * @enddot
 *
 */
class RSocketBase 
: public SNotCopyable, public StdIdMember,
  public RObjectWithEvents<SocketBaseAxis>
{
  friend class RSocketAddress;
  friend std::ostream&
	 operator<< (std::ostream&, const RSocketBase&);

  DECLARE_EVENT(SocketBaseAxis, io_ready);
  DECLARE_EVENT(SocketBaseAxis, bound);
  DECLARE_EVENT(SocketBaseAxis, closed);
  DECLARE_EVENT(SocketBaseAxis, connection_timed_out)
  DECLARE_EVENT(SocketBaseAxis, connection_refused)
  DECLARE_EVENT(SocketBaseAxis, destination_unreachable)
  DECLARE_EVENT(SocketBaseAxis, address_already_in_use)

public:
  DECLARE_STATES(SocketBaseAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, io_ready);
  DECLARE_STATE_CONST(State, bound);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, connection_timed_out);
  DECLARE_STATE_CONST(State, connection_refused);
  DECLARE_STATE_CONST(State, destination_unreachable);
  DECLARE_STATE_CONST(State, address_already_in_use);

  virtual ~RSocketBase () 
  {
    LOG_DEBUG(log, "Close fd " << fd);
    ::close(fd);
  }

  //! A socket file descriptor.
  const SOCKET fd;

  //! Binds the socket to its address.
  virtual void bind() {};

  //! Closes an outbound part
  virtual void ask_close_out() = 0;

  //! Asks to close everything
  void ask_close();

  virtual std::string universal_id() const
  {
    return StdIdMember::universal_id();
  }
  
  CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }

  const RSocketAddress* get_address() const
  {
    return address;
  }

  Event is_construction_complete_event;

  RSocketRepository *const repository;

protected:
  const CompoundEvent is_terminal_state_event = {
    is_connection_timed_out_event,
    is_connection_refused_event,
    is_destination_unreachable_event,
    is_address_already_in_use_event,
    is_closed_event  
  };

  //! A socket address.
  const RSocketAddress* address = nullptr;
  
  // List of all ancestors terminal events. Each derived
  // (with a public virtual base) class appends its
  // terminal event here from its constructor.
  //std::list<CompoundEvent> ancestor_terminals;

public:
  // TODO
  //! Temporary: list of threads terminal events
  std::list<CompoundEvent> threads_terminals;

protected:
  //! This type is only for repository creation
  RSocketBase (const ObjectCreationInfo& oi,
               const RSocketAddress& addr);
  
  //! set blocking mode
  virtual void set_blocking (bool blocking);

  //! get blocking mode
  //virtual bool get_blocking () const;

  //virtual void process_error(int error);

private:
  typedef Logger<RSocketBase> log;
};

std::ostream&
operator<< (std::ostream&, const RSocketBase&);

class SocketThread: public RThread<std::thread>
{ 
public:
  struct Par : public RThread<std::thread>::Par
  {
    RSocketBase* socket;

  Par(RSocketBase* sock, Event* ext_terminated = 0) 
    : RThread<std::thread>::Par(ext_terminated),
      socket(sock) {}

    RThreadBase* transform_object
      (const RThreadBase*) const
    { THROW_NOT_IMPLEMENTED; }
  };

  typedef RThread<std::thread>::RepositoryType 
    RepositoryType;

protected:
  RSocketBase* socket;

SocketThread(const ObjectCreationInfo& oi, const Par& p) 
  : RThread<std::thread>(oi, p), socket(p.socket) 
  { assert(socket); }
};

class SocketThreadWithPair: public SocketThread
{ 
public:
  SOCKET get_notify_fd() const
  {
    return sock_pair[ForNotify];
  }

protected:
  //! indexing sock_pair sockets
  enum {ForSelect = 0, ForNotify = 1};

  //! the pair for ::select call waking-up
  int sock_pair[2];

  SocketThreadWithPair
    (const ObjectCreationInfo& oi, const Par& p);
  ~SocketThreadWithPair();
};


/*=============================*/
/*========== RSocket ==========*/
/*=============================*/

/**
 * A real socket, for example
 * RSocket<ClientSideSocket, InOutSocket, TCPSocket>
 */
template<class... Bases>
class RSocket : public Bases...
{
  template<class...>
    friend RSocketBase* RSocketAllocator
    (const ObjectCreationInfo& oi,
     const RSocketAddress& addr);

public:
  std::string universal_id() const override
  {
    return RSocketBase::universal_id();
  }

  std::string object_name() const override
  {
    return SFORMAT("RSocket:" << this->fd);
  }

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    ax.state_changed
      (this, object, state_ax, new_state);
  }
  
  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  { 
    return ax.current_state(this);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return ax.current_state(this);
  }

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RSocketBase::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }
  
  /*void state_hook
    (AbstractObjectWithStates* object) override
  {
    THROW_PROGRAM_ERROR;
    }*/

protected:
  RSocket(const ObjectCreationInfo& oi,
          const RSocketAddress& addr);

  //! wait all parts terminated and set
  //! its own is_terminal_state_event. 
  ~RSocket();

  DEFAULT_LOGGER(RSocket<Bases...>);
};

//=================================================
// Functions to make an appropriate RSocket type by
// enum parameters 

inline RSocketBase* RSocketAllocator0
(SocketSide side,
 NetworkProtocol protocol,
 IPVer ver,
 const ObjectCreationInfo& oi,
 const RSocketAddress& addr);
    
//template<class Side, class... Others>
//struct RSocketBase* RSocketAllocator1;

template<class Side, class Protocol, class... Others>
  inline RSocketBase* RSocketAllocator2
  (IPVer ver, 
   const ObjectCreationInfo& oi,
   const RSocketAddress& addr);

template<class... Bases>
inline RSocketBase* RSocketAllocator
(const ObjectCreationInfo& oi,
 const RSocketAddress& addr);


class RSocketRepository :
  public Repository
    <RSocketBase, RSocketAddress, std::map, SOCKET>
{
public:
  typedef Repository
    <RSocketBase, RSocketAddress, std::map, SOCKET>
    Parent;

  RThreadFactory *const thread_factory;

  //! Create repository (RSocket factory).
  //! @param id The repository name.
  //! @param reserved The initially allocated space.
  //! @param max_input_packet The max input packet size
  //! (logical piece of data defined in upper protocol,
  //! not a size of tcp/ip packet).
  //! @param tf The thread factory.
  RSocketRepository(
    const std::string& id,
    size_t max_input_packet,
    RThreadFactory *const tf =
      &StdThreadRepository::instance()
    );

  //! Set timeout for usecs. If usecs < 0 - do not use
  //! timeout.
  void set_connect_timeout_u(int64_t usecs);

  bool is_use_connect_timeout() const
  { 
    return use_connect_timeout; 
  }

  //! Sets the backlob parameter to listen(2)
  void set_pending_connections_queue_size(int size)
  {
    pending_connections_queue_size = size;
  }

  int get_pending_connections_queue_size() const
  {
    return pending_connections_queue_size;
  }

  //! Return a copy of timeval
  const std::unique_ptr<struct timeval> 
    connect_timeout_timeval() const;

  //! Return a max input packet size
  const size_t get_max_input_packet_size() const
  { return max_input_packet_size; }

protected:
  struct timeval connect_timeout;
  std::atomic<int> pending_connections_queue_size;
  std::atomic<bool> use_connect_timeout;
  std::atomic<size_t> max_input_packet_size;
};

//! @}

}
#endif
