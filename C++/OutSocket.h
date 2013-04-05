// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_OUTSOCKET_H_
#define CONCURRO_OUTSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

class OutSocketStateAxis : public StateAxis {};

class OutSocket
: public RObjectWithEvents<OutSocketStateAxis>,
  virtual public RSocketBase
{
public:
  DECLARE_STATES(OutSocketStateAxis, State);
  DECLARE_STATE_CONST(State, wait_you);
  DECLARE_STATE_CONST(State, busy);
  DECLARE_STATE_CONST(State, closed);

  //! Doing ::select and signalling wait_you.
  void run();

protected:
  OutSocket();
};


#endif
