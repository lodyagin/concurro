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

    Par() : sar(new RSocketAddressRepository)
	 {
		assert(sar);
	 }

	 virtual ~Par() {}
	 virtual RSocketConnection* create_derivation
	   (const ObjectCreationInfo& oi) const = 0;
	 virtual RSocketConnection* transform_object
	   (const RSocketConnection*) const
	 { THROW_NOT_IMPLEMENTED; }
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
  };

  virtual ~RSocketConnection() {}

  virtual RSocketConnection&
  operator<< (const std::string) = 0;

  virtual RSocketConnection& 
  operator<< (RSingleBuffer&&) = 0;

  //! Non-blocking close
  virtual void ask_close() = 0;

protected:
  RSocketConnection
	 (const ObjectCreationInfo& oi,
	  const Par& par);

  RSocketRepository *const socket_rep;
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
		(const ObjectCreationInfo& oi) const
	 { 
		return new ConnectionThread(oi, *this); 
	 }

	 Connection* con;
  };

protected:
  ConnectionThread(const ObjectCreationInfo& oi, 
						 const Par& p)
	 : SocketThread(oi, p), con(p.con) {}
  ~ConnectionThread() { destroy(); }

  void run()
  {
	 ThreadState::move_to(*this, workingState);
	 con->run();
  }

  Connection* con;
};

//! A connection which always uses only one socket
class RSingleSocketConnection : public RSocketConnection
{
public:
  typedef ConnectionThread<RSingleSocketConnection>
	 Thread;
  friend Thread;

  struct Par : public virtual RSocketConnection::Par
  {
	 RSocketAddress* sock_addr;

	 Par()
		: sock_addr(0) // desc. must init it by an address
		               // from the address repository (sar)
	 {}

	 std::unique_ptr<SocketThread::Par> 
	 get_thread_par(RSingleSocketConnection* c) const
	 {
		return std::unique_ptr<Thread::Par>
		  (new Thread::Par(c));
	 }
  };

  template<NetworkProtocol proto, IPVer ip_ver>
  struct InetClientPar 
  : public Par,
    public RSocketConnection::InetClientPar<proto, ip_ver>
  {
    InetClientPar(const std::string& a_host,
						uint16_t a_port) 
		: RSocketConnection::InetClientPar<proto, ip_ver>
		     (a_host, a_port)
	 {}
  };

  RSocketConnection& operator<< (const std::string);
  RSocketConnection& operator<< (RSingleBuffer&&);

  // TODO move to separate (client side) class
  void ask_connect();

  void ask_close();

  RSocketBase* get_socket()
  {
	 return socket;
  }
  
protected:
  RSingleSocketConnection
	 (const ObjectCreationInfo& oi,
	  const Par& par);
  ~RSingleSocketConnection();

  // <NB> not virtual
  void run();

  //RSocketBase* socket;
  InSocket* socket;
  SocketThread* thread;

public:
  //! A current window
  RConnectedWindow win;
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

  RConnectionRepository(const std::string& id,
								size_t reserved,
								RSocketRepository* sr)
	 : Parent(id, reserved), sock_rep(sr)
  {}

  RSocketRepository *const sock_rep;
};

#endif

