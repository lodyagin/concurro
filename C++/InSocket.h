// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_INSOCKET_H_
#define CONCURRO_INSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

class InSocketStateAxis : public StateAxis {};

class InSocket
: public RObjectWithEvents<InSocketStateAxis>,
  virtual public RSocketBase
{
public:
  DECLARE_STATES(InSocketStateAxis, State);
  DECLARE_STATE_CONST(State, new_data);
  DECLARE_STATE_CONST(State, empty);
  DECLARE_STATE_CONST(State, closed); // a reading side
                                      // was closed

protected:
  ~InSocket();
  
  //! Doing ::select and signalling new_data.
  class Thread : public SocketThread
  {
  public:
    void run(); 

  protected:
    Thread(const ObjectCreationInfo& oi, const Par& p) 
    : SocketThread(oi, p), 
      socket(dynamic_cast<InSocket*>(p.socket)) 
	{ assert(socket); }

    InSocket* socket;
  };

  //! The last received data
  RSingleBuffer msg;

  //! Actual size of a socket internal read buffer + 1.
  size_t socket_rd_buf_size;

protected:
  InSocket();
};

#endif
