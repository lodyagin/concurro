// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKETCONNECTION_H_
#define CONCURRO_RSOCKETCONNECTION_H_

#include "Repository.h"
#include "RSocket.h"
#include "ClientSocket.h"
#include "RWindow.h"
#include <string>
#include <memory>
#include <vector>
#include <iostream>

class RSocketConnection 
: //public RObjectWithEvents<ConnectionAxis>,
public StdIdMember
{
public:
  struct Par {
    //! Available addresses for socket(s)
    std::unique_ptr<RSocketAddressRepository> sar;
    size_t win_rep_capacity;

    Par() 
    : sar(new RSocketAddressRepository),
      win_rep_capacity(3)
      {
        assert(sar);
      }

    Par(Par&& par)
    : sar(std::move(par.sar)),
      win_rep_capacity(par.win_rep_capacity),
      socket_rep(std::move(par.socket_rep)) {}

    virtual ~Par() {}
    virtual RSocketConnection* create_derivation
    (const ObjectCreationInfo& oi) const = 0;
    virtual RSocketConnection* transform_object
    (const RSocketConnection*) const
      { THROW_NOT_IMPLEMENTED; }

    //! Must be inited from derived classes.
    //! A scope of a socket repository can be any: per
    //! connection, per connection type, global etc.
    mutable std::unique_ptr<RSocketRepository> socket_rep;
  };

  //! Parameters to create client side of an Internet
  //! connection. 
  template<NetworkProtocol proto, IPVer ip_ver>
    struct InetClientPar : public virtual Par
  {
    std::string host;
    uint16_t port;

    InetClientPar(const std::string& a_host,
                  uint16_t a_port) 
    : host(a_host), port(a_port)
    {
      sar->create_addresses<proto, ip_ver>
        (host, port);
    }

    InetClientPar(InetClientPar&& par)
      : Par(std::move(par)),
      host(std::move(par.host)),
      port(par.port) {}
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

  //! A window repostiry
  RConnectedWindowRepository win_rep;

protected:
  RSocketConnection
    (const ObjectCreationInfo& oi,
     const Par& par);

  std::shared_ptr<RSocketRepository> socket_rep;
};

std::ostream&
operator<< (std::ostream&, const RSocketConnection&);

class InSocket;

template<class Connection>
class ConnectionThread : public SocketThread
{
public:
  struct Par : public SocketThread::Par
  {
  Par(Connection* c) 
    : SocketThread::Par(c->socket), con(c)
    {
      thread_name = SFORMAT(
        typeid(Connection).name() << socket->fd);
    }

    RThreadBase* create_derivation
      (const ObjectCreationInfo& oi) const override
    { 
      return new ConnectionThread(oi, *this); 
    }

    Connection* con;
  };

protected:
  ConnectionThread(const ObjectCreationInfo& oi, 
                 const Par& p)
  : SocketThread(oi, p), con(p.con) 
  {
    con->socket->threads_terminals.push_back
      (this->is_terminal_state());
  }

  ~ConnectionThread() { destroy(); }

  void run()
  {
    ThreadState::move_to(*this, workingState);
    con->run();
  }

  Connection* con;
};

DECLARE_AXIS(ClientConnectionAxis, ClientSocketAxis);

//! A connection which always uses only one socket
class RSingleSocketConnection 
: public RSocketConnection,
  public RStateSplitter
  <ClientConnectionAxis, ClientSocketAxis>
{
  DECLARE_EVENT(ClientConnectionAxis, aborting);
  DECLARE_EVENT(ClientConnectionAxis, aborted);
  DECLARE_EVENT(ClientConnectionAxis, clearly_closed);

public:
  DECLARE_STATES(ClientConnectionAxis, State);
  DECLARE_STATE_CONST(State, aborting);
  DECLARE_STATE_CONST(State, aborted);
  DECLARE_STATE_CONST(State, clearly_closed);

  typedef ConnectionThread<RSingleSocketConnection>
    Thread;
  friend Thread;

  struct Par : public virtual RSocketConnection::Par
  {
    RSocketAddress* sock_addr;

    Par()
    : sock_addr(0), // descendats must init it by an
      // address
      // from the address repository (sar)
      socket(0) // descendant must create it in
      // create_derivation 
      {}

    Par(Par&& par) 
      : RSocketConnection::Par(std::move(par)),
      sock_addr(par.sock_addr),
      socket(par.socket) {}

    virtual std::unique_ptr<SocketThread::Par> 
      get_thread_par(RSingleSocketConnection* c) const
    {
      return std::unique_ptr<Thread::Par>
        (new Thread::Par(c));
    }

    virtual std::unique_ptr<RConnectedWindow::Par>
      get_window_par(RSocketBase* sock) const
    {
      return std::unique_ptr<RConnectedWindow::Par>
        (new RConnectedWindow::Par(sock));
    }

    mutable RSocketBase* socket;
  };

  template<NetworkProtocol proto, IPVer ip_ver>
    struct InetClientPar 
    : public Par,
    public RSocketConnection::InetClientPar<proto, ip_ver>
  {
    InetClientPar(const std::string& a_host,
                  uint16_t a_port) 
    : RSocketConnection::InetClientPar<proto, ip_ver>
      (a_host, a_port) {}

    InetClientPar(InetClientPar&& par)
      : RSocketConnection::Par(std::move(par)),
                            // virtual base
        Par(std::move(par)),
        RSocketConnection::InetClientPar<proto, ip_ver>
          (std::move(par)) {}
  };

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override;

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
  
  std::string universal_id() const override
  {
    return universal_object_id;
  }

  /*void state_hook
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);*/

protected:
  RSingleSocketConnection
    (const ObjectCreationInfo& oi,
     const Par& par);
  ~RSingleSocketConnection();

  // <NB> not virtual
  void run();

  //RSocketBase* socket;
  InSocket* socket;
  ClientSocket* cli_sock;
  SocketThread* thread;
  RConnectedWindow* in_win;

  A_DECLARE_EVENT(ClientConnectionAxis, 
                  ClientSocketAxis, closed);

protected:
  CompoundEvent is_terminal_state_event;

  DEFAULT_LOGGER(RSingleSocketConnection);

public:
  //! A current window
  //std::unique_ptr<RConnectedWindow> win;
  RConnectedWindow& iw() { return *in_win; }
};

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

  RConnectionRepository(const std::string& id,
                        size_t reserved,
                        RThreadFactory *const tf)
  : Parent(id, reserved), thread_factory(tf)
  {}
};

#endif
