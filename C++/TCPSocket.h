// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_TCPSOCKET_H_
#define CONCURRO_TCPSOCKET_H_

#include "RSocket.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "Logging.h"
#include <netdb.h>

class TCPStateAxis : public StateAxis {};

class TCPSocket : virtual public RSocketBase
, public RObjectWithStates<TCPStateAxis>
{
public:
  DECLARE_STATES(TCPStateAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, in_closed);
  DECLARE_STATE_CONST(State, out_closed);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, syn_sent);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, established);
  DECLARE_STATE_CONST(State, closing);
  DECLARE_STATE_CONST(State, destroyed);

  ~TCPSocket();

  virtual void ask_close();

protected:
  typedef Logger<TCPSocket> log;
  
  //! It is set in the constructor by 
  //! ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  //! Create a TCP socket in a "closed" state.
  TCPSocket();

  class Thread : public SocketThread
  {
  public:
	 PAR_CREATE_DERIVATION(Thread, SocketThread, 
								  RThreadBase)
  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p) {}
	 void run();
  } thread;
};

#endif

