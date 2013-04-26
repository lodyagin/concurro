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
#include <string>
#include <memory>

//class ConnectionAxis : public StateAxis {};

class RSocketConnection 
: //public RObjectWithEvents<ConnectionAxis>,
  public StdIdMember
{
public:
  struct Par {
	 RSocketRepository* socket_rep;

	 //! Available addresses for socket(s)
	 std::unique_ptr<RSocketAddressRepository> sar;

    Par(RSocketRepository* sock_rep) 
	 : socket_rep(sock_rep),
		sar(new RSocketAddressRepository)
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
						uint16_t a_port,
						RSocketRepository* sock_rep) 
		: Par(sock_rep),
		host(a_host), port(a_port)
	 {
		sar->create_addresses<proto, ip_ver>
		  (host, port);
	 }
  };

protected:
  RSocketConnection
	 (const ObjectCreationInfo& oi,
	  const Par& par)
	 : StdIdMember(oi.objectId),
	 socket_rep(par.socket_rep)
  {
	 assert(socket_rep);
  }

  virtual ~RSocketConnection() {}

  RSocketRepository* socket_rep;
};

//! A connection which always uses only one socket
class RSingleSocketConnection : public RSocketConnection
{
public:
  struct Par : public virtual RSocketConnection::Par
  {
	 RSocketAddress* sock_addr;

	 Par(RSocketRepository* sock_rep)
		: RSocketConnection::Par(sock_rep),
	 sock_addr(0) // desc. must init it by an address
		           // from the address repository (sar)
	 {}
  };

  template<NetworkProtocol proto, IPVer ip_ver>
  struct InetClientPar 
  : public Par,
    public RSocketConnection::InetClientPar<proto, ip_ver>
  {
    InetClientPar(const std::string& a_host,
						uint16_t a_port,
						RSocketRepository* sock_rep) 
		: Par(sock_rep),
	     RSocketConnection::InetClientPar<proto, ip_ver>
		     (a_host, a_port, sock_rep)
	 {}
  };

protected:
  RSingleSocketConnection
	 (const ObjectCreationInfo& oi,
	  const Par& par);
  ~RSingleSocketConnection();

  RSocketBase* socket;
};

#endif

