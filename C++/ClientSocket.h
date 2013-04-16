// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_CLIENTSOCKET_H_
#define CONCURRO_CLIENTSOCKET_H_

#include "RSocket.h"

class ClientSocketAxis : public StateAxis {};

class ClientSocket : virtual public RSocketBase,
  public RObjectWithEvents<ClientSocketAxis>
{
  DECLARE_EVENT(ClientSocketAxis, connected)
  DECLARE_EVENT(ClientSocketAxis, connection_timed_out)
  DECLARE_EVENT(ClientSocketAxis, connection_refused)
  DECLARE_EVENT(ClientSocketAxis, destination_unreachable)

public:
  DECLARE_STATES(ClientSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, connecting);
  DECLARE_STATE_CONST(State, connected);
  DECLARE_STATE_CONST(State, connection_timed_out);
  DECLARE_STATE_CONST(State, connection_refused);
  DECLARE_STATE_CONST(State, destination_unreachable);
  DECLARE_STATE_CONST(State, destroyed);

  ~ClientSocket();

  //! Start connection to a server. It can result in
  //! connecting -> {connected, connection_timed_out,
  //! connection_refused and destination_unreachable}
  //! states.
  virtual void ask_connect();

protected:
  ClientSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);

  void process_connect_error(int error);
  
  RThreadFactory* thread_factory;

  class Thread : public SocketThread
  {
  public:
	 struct Par : public SocketThread::Par
	 { 
		Par(RSocketBase* sock) 
		  : SocketThread::Par(sock) {}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new Thread(oi, *this); 
		}
	 };

	 void run();
  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p) {}
	 ~Thread() { destroy(); }
  }* thread;

  DEFAULT_LOGGER(ClientSocket)
};

#endif
