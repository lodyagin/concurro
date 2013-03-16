#pragma once
#include "RSingleSocket.h"
#include "RClientSocketAddress.h"
#include "StateMap.h"

class ConnectionStateAxis : public StateAxis {};

class RSocketTCP : public RSingleSocket
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

  void state (State& state) const
  {
	 state = currentState;
  }

protected:

  void set_state_internal (const State& state)
  {
    currentState = state;
  }

private:
  State currentState;
};
