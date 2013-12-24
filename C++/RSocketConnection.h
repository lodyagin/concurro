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

#ifndef CONCURRO_RSOCKETCONNECTION_H_
#define CONCURRO_RSOCKETCONNECTION_H_

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "Repository.h"
#include "RSocket.h"
#include "RSocketAddress.h"
#include "ListeningSocket.h"
#include "RWindow.h"
#include "RObjectWithThreads.h"

namespace curr {

/**
 * @addtogroup connections
 * @{
 */
class RSocketConnection : public StdIdMember
{
public:
  struct Par 
  {
    Par() {}

    Par(Par&& par) :
      socket_rep(std::move(par.socket_rep)) 
    {}

    virtual ~Par() {}

    virtual RSocketConnection* create_derivation
      (const ObjectCreationInfo& oi) const = 0;

    virtual RSocketConnection* transform_object
      (const RSocketConnection*) const
    { THROW_NOT_IMPLEMENTED; }

    //! Must be inited from derived classes.
    //! A scope of a socket repository can be any: per
    //! connection, per connection type, global etc.
    mutable RSocketRepository* socket_rep;
   //mutable std::unique_ptr<RSocketRepository> socket_rep;
  };

  //! Parameters to create a client side of an Internet
  //! connection. 
  template<NetworkProtocol proto, IPVer ip_ver>
  struct InetClientPar : public virtual Par
  {
    //! Available addresses for socket(s)
    //RSocketAddressRepository* sar;
    const RSocketAddress* sock_addr;

    InetClientPar(RSocketAddress* sa) : sock_addr(sa)
    {
      assert(sock_addr);
    }
  };

  virtual ~RSocketConnection() {}

  virtual RSocketConnection&
    operator<< (const std::string&) = 0;

  virtual RSocketConnection& 
    operator<< (RSingleBuffer&&) = 0;

  //! Non-blocking close
  virtual void ask_close() = 0;

  //! Skip all data (non-blocking)
  virtual void ask_abort() = 0;

protected:
  RSocketConnection
    (const ObjectCreationInfo& oi,
     const Par& par);

  RSocketRepository* socket_rep;
};

std::ostream&
operator<< (std::ostream&, const RSocketConnection&);

DECLARE_AXIS_TEMPL(SocketConnectionAxis, 
                   RSocketBase,
                   T::State::axis);

/**
 * A connection which always uses only one socket defined
 * as a template parameter. There is an internal input
 * thread which reads packets into iw() input window.
 *
 * \tparam Connection is a final descendant - the actual
 * connection class.
 * \tparam Socket either ClientSocket or server (accepted)
 * socket. It defines the connection side.
 * \tparam Threads pars of (additional) threads that serve
 * this connection; it is mandatory for a server side of
 * the connection.
 */
template<class Connection, class Socket, class... Threads>
class RSingleSocketConnection 
: public RSocketConnection,
  public RStateSplitter
  <
    SocketConnectionAxis<Socket>, 
    typename Socket::State::axis
  >,
  public RObjectWithThreads<Connection>
{
  DECLARE_EVENT(SocketConnectionAxis<Socket>, aborting);
  DECLARE_EVENT(SocketConnectionAxis<Socket>, aborted);
  DECLARE_EVENT(SocketConnectionAxis<Socket>, 
                clearly_closed);
  DECLARE_EVENT(SocketConnectionAxis<Socket>, io_ready);
  DECLARE_EVENT(SocketConnectionAxis<Socket>, closed);

public:
  //! @cond
  DECLARE_STATES(SocketConnectionAxis<Socket>, State);
  DECLARE_STATE_CONST(typename State, aborting);
  DECLARE_STATE_CONST(typename State, aborted);
  DECLARE_STATE_CONST(typename State, clearly_closed);
  //! @endcond

  typedef RStateSplitter
  <
    SocketConnectionAxis<Socket>, 
    typename Socket::State::axis
  > Splitter;

  struct Par : public virtual RSocketConnection::Par
  {
    mutable RSocketBase* socket;

    Par() :
      //sock_addr(0), // descendats must init it by an
      // address
      // from the address repository (sar)
      socket(0) // descendant must create it in
      // create_derivation 
    {}

    Par(Par&& par) :
      RSocketConnection::Par(std::move(par)),
      //sock_addr(par.sock_addr),
      socket(par.socket) {}

    virtual std::unique_ptr<RConnectedWindow<SOCKET>::Par>
    get_window_par(RSocketBase* sock) const
    {
      return std::unique_ptr<RConnectedWindow<SOCKET>::Par>
        (new RConnectedWindow<SOCKET>::Par(sock->fd));
    }
  };

  template<NetworkProtocol proto, IPVer ip_ver>
  struct InetClientPar :
    public Par,
    public RSocketConnection::InetClientPar<proto, ip_ver>
  {
    //! The max input packet size
    //! (logical piece of data defined in upper protocol,
    //! not a size of tcp/ip packet).
    size_t max_input_packet = -1;

    //! The connection timeout in usecs
    uint64_t max_connection_timeout = 3500000;

    InetClientPar
      (RSocketAddress* sa, size_t max_packet) 
    :
       RSocketConnection::InetClientPar<proto, ip_ver>(sa),
       max_input_packet(max_packet)
    {}

    InetClientPar(InetClientPar&& par)
      : RSocketConnection::Par(std::move(par)),
                            // virtual base
        Par(std::move(par)),
        RSocketConnection::InetClientPar<proto, ip_ver>
          (std::move(par)) {}

    RSocketConnection* create_derivation
      (const ObjectCreationInfo& oi) const override;
  };

  struct ServerPar : Par
  {
    ServerPar(RSocketBase* srv_sock)
    {
      assert(srv_sock);
      this->socket = srv_sock;
      assert(srv_sock->repository);
      this->socket_rep = srv_sock->repository;
    }

    ServerPar(ServerPar&& par)
      : Par(std::move(par))
    {}

    RSocketConnection* create_derivation
      (const ObjectCreationInfo& oi) const
    {
      return new Connection(oi, *this);
    }
  };

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;

  RSocketConnection& operator<< 
    (const std::string&) override;
  RSocketConnection& operator<< 
    (RSingleBuffer&&) override;

  // TODO move to separate (client side) class
  void ask_connect();

  void ask_close() override;

  void ask_abort() override;

  RSocketBase* get_socket()
  {
    return socket;
  }

  CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }
  
protected:
  RSingleSocketConnection
    (const ObjectCreationInfo& oi,
     const Par& par);
  ~RSingleSocketConnection();

  std::atomic<uint32_t>&
  current_state(const curr::StateAxis& ax) override
  {
    return ax.current_state(this);
  }

  const std::atomic<uint32_t>&
  current_state(const curr::StateAxis& ax) const override
  {
    return ax.current_state(this);
  } 

  MULTIPLE_INHERITANCE_DEFAULT_EVENT_MEMBERS;

  // <NB> not virtual
  void run();

  InSocket* socket;
  //SocketThread* thread;
  RConnectedWindow<SOCKET>* in_win;

protected:
  CompoundEvent is_terminal_state_event;

  DEFAULT_LOGGER(RSingleSocketConnection);

public:
  //! A current window
  RConnectedWindow<SOCKET>& iw() { return *in_win; }
};

namespace connection { 

//! Allows define a server thread as a virtual method
//! override.
template<class Connection>
class abstract_server
{
public:
  struct Par : ObjectFunThread<Connection>::Par
  {
    Par(SOCKET fd) : 
      ObjectFunThread<Connection>::Par
        (SFORMAT
          (typeid(abstract_server<Connection>). name() 
           << fd),
         [](abstract_server& obj)
         {
           obj.server_run();
         }
        )
     {}
  };

  virtual void server_run() = 0;
};

namespace single_socket {

//! This class defines a server thread, you can inherit
//! from it and overwrite server_run().
template<class Connection, class Socket, class... Threads>
class server :
  public RSingleSocketConnection
  <
    Connection, 
    Socket,
    typename abstract_server<Connection>::Par,
    //!< a server thread par
    Threads...
  >,
  public abstract_server<Connection>
{
protected:
  using RSingleSocketConnection
  <
    Connection, 
    Socket,
    typename abstract_server<Connection>::Par,
    //!< a server thread par
    Threads...
  >::RSingleSocketConnection;
};

}}

class RConnectionRepository
: public Repository<
  RSocketConnection, 
  RSocketConnection::Par,
  std::vector,
  size_t>
{
public:
  typedef Repository<
    RSocketConnection, 
    RSocketConnection::Par,
    std::vector,
    size_t> Parent;

  RThreadFactory *const thread_factory;

  RConnectionRepository
    (const std::string& id,
     size_t reserved,
     RThreadFactory *const tf = 
       &StdThreadRepository::instance()
    )
  : Parent(id, reserved), thread_factory(tf)
  {}
};

DECLARE_AXIS(ServerConnectionFactoryAxis, 
             ListeningSocketAxis);

/**
  * A factory of socket server connections based on a
  * single ListeningSocket.
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
template<class Connection>
class RServerConnectionFactory final
  : public RStateSplitter
      <ServerConnectionFactoryAxis, ListeningSocketAxis>,
    public RConnectionRepository
{
public:
  //! Accepts ListeningSocket in the bound state.
  RServerConnectionFactory
    (ListeningSocket* l_sock, size_t reserved);

  CompoundEvent is_terminal_state() const override
  {
    assert(lstn_sock);
    return lstn_sock->is_terminal_state();
    // FIXME & treads.is_terminal_state();
  }

protected:
  //! It is an internal object to prevent states mixing
  class Threads final : public RObjectWithThreads<Threads>
  {
  public:
    Threads(RServerConnectionFactory* o);

    ~Threads() { this->destroy(); }

    CompoundEvent is_terminal_state() const override
    {
      return CompoundEvent();
    }

    RServerConnectionFactory* obj;

  } threads;

  class ListenThread final : public ObjectThread<Threads>
  {
  public:
    struct Par : ObjectThread<Threads>::Par
    {
      Par() : 
        ObjectThread<Threads>::Par
          ("RServerConnectionFactory::Threads::"
           "ListenThread")
      {}

      PAR_DEFAULT_OVERRIDE(StdThread, ListenThread);
    };

  protected:
    REPO_OBJ_INHERITED_CONSTRUCTOR_DEF(
      ListenThread, 
      ObjectThread<Threads>, 
      ObjectThread<Threads>
    );

    void run() override;
  };

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
    THROW_PROGRAM_ERROR;
  }

  ListeningSocket* lstn_sock;
};

//! @}

}
#endif

