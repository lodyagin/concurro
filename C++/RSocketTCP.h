#pragma once
#include "RSingleSocket.h"
#include "RClientSocketAddress.h"
#include "RAbstractConnection.h"
#include "StateMap.h"

class RSocketTCP : public RSingleSocket //, public RAbstractConnection
, public RObjectWithStates<ConnectionStateAxis>
{
public:
  RSocketTCP();
  ~RSocketTCP();

  // TODO declare these also in parents
  virtual void close ();

  /// connect the first available address in addr
  virtual void connect_first (const RClientSocketAddress& addr);

  // TODO add inform about event from tapi-sockets-tcp

  /* TCP states */
  const static State2Idx allStates[];
  const static StateTransition allTrans[];
  typedef RState<RSocketTCP, ConnectionStateAxis, allStates, allTrans> State;
  friend class RState<RSocketTCP, ConnectionStateAxis, allStates, allTrans>;

  const static State closedState;
  const static State listenState;
  const static State syn_sentState;
  const static State establishedState;
  const static State closingState;
  const static State abortedState;
  const static State destroyedState;
};
