#ifndef CONCURRO_CONNECTIONFACTORY_H_
#define CONCURRO_CONNECTIONFACTORY_H_

#include "SSingleton.h"
#include "SEvent.h"
#include "Repository.h"
#include "ThreadRepository.h"
#include "RConnectedSocket.h"

/**
 * An abstract thread-processed connection. It not depends
 * on sockets.  It is abstract base for
 * ClientSideConnectionFactory and
 * ServerSideConnectionFactory. All decisions about
 * connection creation are made by those classes.
 */
template<class Connection, class ConnectionPars>
class ConnectionFactory :
  public SSingleton
    <ConnectionFactory<Connection, ConnectionPars>>
{
public:
  typedef ThreadRepository<Connection, ConnectionPars> 
	 ThreadRepo;

  ConnectionFactory(ThreadRepo* _threads)
    : threads (_threads),
      connectionTerminated (false) // automatic reset
  {
    assert (threads);
  }
  virtual ~ConnectionFactory() = 0;

  void destroy_terminated_connections ();

  REvent connectionTerminated;

protected:
  ThreadRepo* threads;
};

/// It produces connection on server side.
template<class Connection, class ConnectionPars>
class ServerSideConnectionFactory 
  : public ConnectionFactory<Connection, ConnectionPars>
{
public:
  ServerSideConnectionFactory
	 (ThreadRepository<Connection, ConnectionPars>* threads)
	: ConnectionFactory<Connection, ConnectionPars>(threads)
  {}

  /// Create a server side connection.
  /// It will be called from RListeningSocket after
  /// RConnectedSocket creation to start the new
  /// connection thread
  Connection* create_new_connection (RConnectedSocket* cs);

protected:
  /// A connection pars for a server side connection
  virtual ConnectionPars* create_connection_pars 
	 (RConnectedSocket* cs);
};

/**
 * It produces connection on a server side.
 */
template<class Connection, class ConnectionPars>
class ClientSideConnectionFactory 
  : public ConnectionFactory<Connection, ConnectionPars>
{
public:
  ClientSideConnectionFactory
	 (ThreadRepository<Connection, ConnectionPars>* threads)
	: ConnectionFactory<Connection, ConnectionPars>(threads)
  {}

  Connection* create_new_connection
	 (RClientSocketAddress* addr);

protected:
  virtual ConnectionPars* create_connection_pars
	 (RClientSocketAddress*);
};

template<class Connection, class ConnectionPars>
void ConnectionFactory<Connection, ConnectionPars>
//
::destroy_terminated_connections ()
{
    // Found and destroy the terminated thread
    std::list<int> terminated;
    threads->get_object_ids_by_state
      (std::back_inserter (terminated),
       RThreadBase::terminatedState
       );
    std::for_each 
      (terminated.begin (), terminated.end (),
      ThreadRepo::Destroy (*threads));
}

template<class Connection, class ConnectionPars>
Connection* ServerSideConnectionFactory<Connection, ConnectionPars>
//
::create_new_connection(RConnectedSocket* cs)
{
  assert (cs);
  const RSingleprotoSocketAddress& rsa = cs
    -> get_peer_address ();

  LOG_INFO(Logger<LOG::Root>, "New connection from: "
			  << rsa.get_ip () << ':' << rsa.get_port ());

  Connection* rc = NULL;
  ConnectionPars* cp = create_connection_pars(cs);
  rc = this->threads->create_object (*cp);
  delete cp; //!
  rc->start ();

  LOG_DEBUG(Logger<LOG::Root>, "New server side connection is started");
  return rc;
}

template<class Connection, class ConnectionPars>
Connection* ClientSideConnectionFactory<Connection, ConnectionPars>
//
::create_new_connection(RClientSocketAddress* addr)
{
  LOG_INFO(Logger<LOG::Root>, "Try connect to: " << addr);

  Connection* rc = NULL;
  ConnectionPars* cp = create_connection_pars(addr);
  rc = this->threads->create_object (*cp);
  delete cp; //!
  rc->start ();

  LOG_DEBUG(Logger<LOG::Root>, "New client side connection is started");
  return rc;
}

template<class Connection, class ConnectionPars>
ConnectionPars* ServerSideConnectionFactory<Connection, ConnectionPars>
//
::create_connection_pars(RConnectedSocket* cs)
{
  assert (cs);
  ConnectionPars* cp = new ConnectionPars;
  cp->server_socket = cs;
  cp->connectionTerminated = &this->connectionTerminated;
  return cp;
}

template<class Connection, class ConnectionPars>
ConnectionPars* ClientSideConnectionFactory<Connection, ConnectionPars>
//
::create_connection_pars(RClientSocketAddress* addr)
{
  ConnectionPars* cp = new ConnectionPars;
  cp->client_socket_address = addr;
  cp->connectionTerminated = &this->connectionTerminated;
  return cp;
}

#endif
