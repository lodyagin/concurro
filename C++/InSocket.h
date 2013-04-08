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

  virtual void ask_close();

protected:
  typedef Logger<RSocketBase> log;

  //! Doing ::select and signalling new_data.
  class Thread : public SocketThreadWithPair
  {
  public:
	 PAR_CREATE_DERIVATION(Thread, SocketThread, 
								  RThreadBase)
  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThreadWithPair(oi, p) {}
	 void run();
  } thread;

  //! The last received data
  RSingleBuffer msg;

  //! Actual size of a socket internal read buffer + 1.
  size_t socket_rd_buf_size;
  
  InSocket();
  ~InSocket();
};

#endif
