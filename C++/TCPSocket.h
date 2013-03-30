// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_TCPSOCKET_H_
#define CONCURRO_TCPSOCKET_H_

#include "RClientSocketAddress.h"
#include "RSocket.h"
//#include "AbstractConnection.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "Logging.h"
#include <netdb.h>

class TCPStateAxis : public StateAxis {};

class TCPSocket : virtual public RSocketBase
, public RObjectWithStates<TCPStateAxis>
{
public:

  struct Par //: virtual public RSocketBase::Par
  {
	 //! How much to wait a
	 //! connection termination on close()
	 int close_wait_seconds;

    Par(const RSocketAddress& addr, int close_wait) 
	 : //RSocketBase::Par(addr),
        close_wait_seconds(close_wait) {}
  };

  //!
  TCPSocket(int close_wait_seconds);
  ~TCPSocket();

  void close();

  // TODO add inform about event from tapi-sockets-tcp

  DECLARE_STATES(TCPStateAxis, State);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, syn_sent);
  DECLARE_STATE_CONST(State, established);
  DECLARE_STATE_CONST(State, closing);
  DECLARE_STATE_CONST(State, aborted);
  DECLARE_STATE_CONST(State, destroyed);

protected:

  //! Create a TCP socket in a "closed" state.
  TCPSocket(const ObjectCreationInfo&, const Par&);

  typedef Logger<TCPSocket> log;
  
  //! It is set in the constructor by 
  //! ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  //! how much wait closing the socket
  int close_wait_secs;

  //void connect_first(const RClientSocketAddress& addr);
};

#endif

