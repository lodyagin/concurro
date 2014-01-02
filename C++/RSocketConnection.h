/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
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
#include <streambuf>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>

#include "Repository.h"
#include "RSocket.h"
#include "OutSocket.h"
#include "RSocketAddress.h"
#include "ListeningSocket.h"
#include "RWindow.h"
#include "RObjectWithThreads.h"

namespace curr {

namespace connection { 

/**
 * @addtogroup connections
 * @{
 */
class abstract_connection : public StdIdMember
{
  template<class CharT, class Traits>
  friend class basic_streambuf;

public:
  struct Par 
  {
    //! The connection timeout in usecs
    uint64_t max_connection_timeout = 3500000;

    //! The max packet size
    //! (logical piece of data defined in the upper
    //! protocol, not a size of tcp/ip packet).
    size_t max_packet_size;

    Par(size_t max_packet) :
      max_packet_size(max_packet)
    {}

    PAR_DEFAULT_ABSTRACT(abstract_connection);
  };

  virtual ~abstract_connection() {}

  virtual void ask_connect() = 0;

  //! Non-blocking close
  virtual void ask_close() = 0;

  //! Skip all data (non-blocking)
  virtual void ask_abort() = 0;

  //! The max packet size
  //! (logical piece of data defined in the upper protocol,
  //! not a size of tcp/ip packet). Mast be matched on
  //! client and server. For server side
  //! connections it is got from ListeningSocket.
  const size_t max_packet_size;

protected:
  abstract_connection
    (const ObjectCreationInfo& oi,
     const Par& par);

  //! Writes data from out_buf. Returns false if the
  //! connection is closed. It can blocks.
  virtual bool push_out() = 0;

  //! Reads data in in_buf. Returns false if there is no
  //! data and the connection is closed. It can blocks.
  virtual bool pull_in() = 0;

  //! The buffer with an input message
  RSingleBuffer in_buf;

  //! The buffer for an output message
  RSingleBuffer out_buf;
};

std::ostream&
operator<< (std::ostream&, const abstract_connection&);

class repository
: public Repository<
  abstract_connection, 
  abstract_connection::Par,
  std::vector,
  size_t>
{
public:
  typedef Repository<
    abstract_connection, 
    abstract_connection::Par,
    std::vector,
    size_t> Parent;

  RThreadFactory *const thread_factory;

  repository
    (const std::string& id,
     size_t reserved,
     RThreadFactory *const tf = 
       &StdThreadRepository::instance()
    )
  : Parent(id, reserved), thread_factory(tf)
  {}
};

/**
  * A connection-based streambuf.
  */
template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
class basic_streambuf : 
  public std::basic_streambuf<CharT, Traits>
{
public:
  typedef typename Traits::int_type int_type;
  typedef typename Traits::pos_type pos_type;
  typedef typename Traits::off_type off_type;

  basic_streambuf(abstract_connection* con) :
    c(con)
  {
    assert(c);
    assert(c->max_packet_size > 0);

    this->setp(nullptr, nullptr, nullptr);
    this->setg(nullptr, nullptr, nullptr);
  }

protected:
  int sync() override;

  int_type underflow() override;

  int_type overflow(int_type ch = Traits::eof()) override;
	
  abstract_connection* c;
};


struct stream_marker {};
struct bulk_marker {};

/**
  * A connection for fast block reading (without copying
  * memory). It uses RWindow for direct access to input
  * packets.
  */
template<template<class...> class Parent, class... Ts>
class bulk : 
  public Parent
  <
    Ts..., 
    RunProviderPar<bulk<Parent, Ts...>>
  >,
  std::enable_if<
    !std::is_base_of<stream_marker, Parent<Ts...>>::value
    && std::is_base_of 
      <with_threads_marker, Parent<Ts...>>::value,
    bulk_marker
  >::type
{
  friend class RunProviderPar<bulk<Parent, Ts...>>;

public:
  typedef Parent<
    Ts..., 
    RunProviderPar<bulk<Parent, Ts...>>
  > ParentT;
  typedef typename ParentT::Par Par;

  //! A current window
  RConnectedWindow<SOCKET>& iw() { return *in_win; }

  //! Send the message
  template<class Buffer>
  void send(Buffer&& msg)
  {
    this->out_buf.move(&msg);
    this->push_out();
  }

protected:
  bulk(const ObjectCreationInfo& oi, const Par& par);

  void run();

  RConnectedWindow<SOCKET>* in_win;
};

template<
  template<class...> class Parent,
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Enable = void,
  class... Ts
>
class stream;

template<
  template<class...> class Parent,
  class CharT,
  class Traits,
  class... Ts
>
class stream
<
  Parent, CharT, Traits,
  typename std::enable_if<
    !std::is_base_of<bulk_marker, Parent<Ts...>>::value
    && std::is_base_of
      <with_threads_marker, Parent<Ts...>>::value
  >::type,
  Ts...
> 
: public Parent<
    Ts..., 
    RunProviderPar<stream<Parent, std::true_type, Ts...>>
  >,
  stream_marker
{
public:
  typedef Parent<
    Ts..., 
    RunProviderPar<stream<Parent, std::true_type, Ts...>>
  > ParentT;
};

struct server_marker {};

//! This class defines a server thread, you can inherit
//! from it and overwrite run_server().
template<
  template<template<class...> class, class...> class Parent2,
  template<class...> class Parent1,
  class... Ts
>
class server :
  public Parent2
  <
    Parent1,
    Ts..., 
    RunProviderPar<server<Parent2, Parent1, Ts...>>
  >,
  std::enable_if<
    !std::is_base_of
      <server_marker, Parent2<Parent1, Ts...>>::value
    && std::is_base_of
      <with_threads_marker, Parent2<Parent1, Ts...>>::value,
    server_marker
  >::type
{
public:
  typedef Parent2<
    Parent1,
    Ts..., 
    RunProviderPar<server<Parent2, Parent1, Ts...>>
  > ParentT;
  typedef typename ParentT::Par Par;

protected:
  server(const ObjectCreationInfo& oi, const Par& par);

  virtual void run_server() = 0;
  void run() { this-> run_server(); }
};

namespace socket {

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
class connection 
: public abstract_connection,
  public RStateSplitter
  <
    SocketConnectionAxis<Socket>, 
    typename Socket::State::axis
  >
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

  //! Client and server side connections differ by a
  //! socket type passed to the Par constructor.
  struct Par : abstract_connection::Par
  {
    RSocketBase* socket;

    Par(RSocketBase* sock) 
    : 
      abstract_connection::Par
        (sock->repository
          -> get_max_input_packet_size()),
      socket(sock),
      socket_rep(sock->repository)
    {
      assert(socket);
      assert(socket_rep);
    }

    abstract_connection* create_derivation
      (const ObjectCreationInfo& oi) const override
    {
      return new Connection(oi, *this);
    }

    virtual std::unique_ptr<RConnectedWindow<SOCKET>::Par>
    get_window_par(RSocketBase* sock) const
    {
      return std::unique_ptr<RConnectedWindow<SOCKET>::Par>
        (new RConnectedWindow<SOCKET>::Par(sock->fd));
    }

  //protected:
    //! A scope of a socket repository can be any: per
    //! connection, per connection type, global etc.
    RSocketRepository* socket_rep;
  };

#if 0
  //! A client side connection par.
  template<NetworkProtocol proto, IPVer ip_ver>
  struct InetClientPar : Par
  {
    //! Available addresses for socket(s)
    const RSocketAddress* sock_addr;

    InetClientPar(RSocketAddress* sa, size_t max_packet) :
      Par(max_packet),
      sock_addr(sa)
    {
      assert(sock_addr);
    }

    InetClientPar(InetClientPar&& par)
      : abstract_connection::Par(std::move(par)),
                            // virtual base
        Par(std::move(par)),
        abstract_connection::InetClientPar<proto, ip_ver>
          (std::move(par)) {}

    abstract_connection* create_derivation
      (const ObjectCreationInfo& oi) const override;
  };

  //! A server side connection par.
  struct ServerPar : Par
  {
    ServerPar(RSocketBase* srv_sock) :
      Par(srv_sock->repository
          -> get_max_input_packet_size()
         )
    {
      assert(srv_sock);
      this->socket = srv_sock;
      assert(srv_sock->repository);
      this->socket_rep = srv_sock->repository;
    }

    ServerPar(ServerPar&& par)
      : Par(std::move(par))
    {}

    abstract_connection* create_derivation
      (const ObjectCreationInfo& oi) const
    {
      return new Connection(oi, *this);
    }
  };
#endif

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;

  void ask_connect() override;

  void ask_close() override;

  void ask_abort() override;

  /*RSocketBase* get_socket()
  {
    return socket;
  }*/

  /*CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }*/

protected:
  connection
    (const ObjectCreationInfo& oi,
     const Par& par);
  ~connection();

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

  bool push_out() override;

  bool pull_in() override;

  // <NB> not virtual
  void run();

  RSocketRepository* socket_rep;

  //TODO can be nullptr for output only channels
  InSocket* in_sock;

  //! OutSocket*, can be nullptr if it is input only
  //! channel. 
  OutSocket* out_sock;

  CompoundEvent is_terminal_state_event;

  DEFAULT_LOGGER(connection);
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
class server_factory final :
  public RStateSplitter
    <ServerConnectionFactoryAxis, ListeningSocketAxis>,
  public repository
{
public:
  //! Accepts ListeningSocket in the bound state.
  server_factory
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
    Threads(server_factory* o);

    ~Threads() { this->destroy(); }

    CompoundEvent is_terminal_state() const override
    {
      return CompoundEvent();
    }

    server_factory* obj;

  } threads;

  class ListenThread final : public ObjectThread<Threads>
  {
  public:
    struct Par : ObjectThread<Threads>::Par
    {
      Par() : 
        ObjectThread<Threads>::Par
          ("curr::connection::server_factory::Threads::"
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

} // namespace socket
} // namespace connection

//! @}

}
#endif

