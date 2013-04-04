#ifndef CONCURRO_TCPCONNECTION_H_
#define CONCURRO_TCPCONNECTION_H_

#include "AbstractConnection.h"
#include "TCPSocket.h"
#include "RObjectWithStates.h"
#include "ConnectionRepository.h"
#include "ThreadRepository.h"
#include "RThread.h"
#include <limits>
#include <ostream>

template<class Thread>
class TCPConnection 
 : public AbstractConnection, public Thread
{
public:

  struct Par 
	 : public AbstractConnection::Par,
	   public Thread::Par
  {
	 TCPConnection<Thread>* transform_object
		(TCPConnection<Thread>* from) const
	 {
		return from; // no transformation
	 }

	 virtual TCPConnection<Thread>* create_derivation
		(const ObjectCreationInfo&) const = 0;

	 virtual TCPConnection<Thread>* transform_object
		(const RThreadBase*) const
	 {
		THROW_EXCEPTION(SException, "Not implemented");
	 }
  };

  struct ServerSidePar : public Par
  {
	 /// A socket connected to a client.
	 RSocketTCP* server_socket;

    ServerSidePar() : server_socket (0) {}

	 TCPConnection<Thread>* create_derivation
		(const ObjectCreationInfo&) const;
  };

  struct ClientSidePar : public Par
  {
	 /// An address to connect to.
	 RClientSocketAddress* client_socket_address;
	 /// how much to wait a connection termination on
    /// close()
	 int close_wait_seconds;

    ClientSidePar() 
     : client_socket_address (0),
		 close_wait_seconds(std::numeric_limits<int>::max())
	 {}

	 TCPConnection<Thread>* create_derivation
		(const ObjectCreationInfo&) const;
  };

  /* overrided */
  void ask_open() { Thread::start(); }
  void ask_close() { Thread::stop(); }

  /// Delegate state to the socket
  void state(ConnectionStateAxis& state) const
  {
	 this->socket->state(state);
  }

  bool state_is(const ConnectionStateAxis& state) const
  {
	 this->socket->state_is(state);
  }

  std::string universal_id() const
  {
	 return Thread::universal_id();
  }

protected:
  typedef Logger<TCPConnection> log;

  TCPConnection
	 (const ObjectCreationInfo&, const ServerSidePar&);
  TCPConnection
	 (const ObjectCreationInfo&, const ClientSidePar&);

  /*void set_state_internal(const ConnectionStateAxis& state)
  {
	 this->socket->set_state_internal(state);
	 }*/

  /* overrided */
  void run();

  //void process_input(

  // FIXME need use smart ptr, coz when client side it is
  // created, but when server side it is inherited
  RSocketTCP* socket;

  /// It is used for client side connections only
  // FIXME memory leak (need to make copyable and not use
  // pointer). 
  RClientSocketAddress* client_socket_address;
};

template<class Thread>
std::ostream& operator<< 
  (std::ostream& out, const TCPConnection<Thread>& c)
{
  out << "TODO: operator<< for TCPConnection "
	 "is not implemented";
  return out;
}

template<class Thread>
TCPConnection<Thread>::TCPConnection
  (const ObjectCreationInfo& oi, const ServerSidePar& par)
: AbstractConnection
    (oi.objectId, par.connectionTerminated),
  Thread(oi, par),
  socket(par.socket),
  client_socket_address(0)
{
  assert (socket);
}

template<class Thread>
TCPConnection<Thread>::TCPConnection
  (const ObjectCreationInfo& oi, const ClientSidePar& par)
: AbstractConnection
    (oi.objectId, par.connectionTerminated),
  Thread(oi, par),
  socket(new RSocketTCP(par.close_wait_seconds)),
  client_socket_address(par.client_socket_address)
{
  assert (client_socket_address);
}

template<class Thread>
TCPConnection<Thread>* 
TCPConnection<Thread>::ClientSidePar
//
::create_derivation(const ObjectCreationInfo& oi) const
{
  return new TCPConnection<Thread>(oi, *this);
}

template<class Thread>
void TCPConnection<Thread>::run()
{
  socket->connect_first(*client_socket_address);
  
  socket->set_blocking(false);
  REvent* socketEvent = socket->get_event_object();
//  sleep(2);
  for (;;)
  {
	 LOG_DEBUG(Logger<LOG::Concurrency>,
				  "socketEvent->wait()");
	 socketEvent->wait();
	 int32_t events = socket->get_events(true);
	 LOG_DEBUG(log, "Socket events: " << events);
  }
}

#endif
