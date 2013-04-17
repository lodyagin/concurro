// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_TCPSOCKET_H_
#define CONCURRO_TCPSOCKET_H_

#include "RSocket.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "Logging.h"
#include <netdb.h>

class TCPAxis : public StateAxis {};

class TCPSocket : virtual public RSocketBase
, public RObjectWithEvents<TCPAxis>
{
  DECLARE_EVENT(TCPAxis, closed);

public:
  DECLARE_STATES(TCPAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, in_closed);
  DECLARE_STATE_CONST(State, out_closed);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, syn_sent);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, established);
  DECLARE_STATE_CONST(State, closing);

  ~TCPSocket();

  const CompoundEvent is_terminal_state() const
  {
	 return is_closed_event;
  }

protected:
  typedef Logger<TCPSocket> log;
  
  //! It is set in the constructor by 
  //! ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  //! Create a TCP socket in a "closed" state.
  TCPSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);

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

  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p) {}
	 ~Thread() { destroy(); }
	 void run();
  }* thread;
};

#endif

