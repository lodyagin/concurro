#pragma once
#include "RSingleSocket.h"
#include "RClientSocketAddress.h"
#include "AbstractConnection.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "Logging.h"
#include <netdb.h>

class RSocketTCP : public RSingleSocket
, public RObjectWithStates<ConnectionStateAxis>
{
public:
  /// Create a TCP socket in "closed" state.
  /// \param close_wait_seconds how much to wait a connection 
  ///        termination on close()
  RSocketTCP(int close_wait_seconds);
  ~RSocketTCP();

  // TODO declare these also in parents
  virtual void close ();

  //! Connect the first available address in addr. States
  //! transitions are, for get_blocking() == true:
  //! ->syn_sent, for get_blocking() == false: 
  //! ->syn_sent[->established]
  virtual void connect_first (const RClientSocketAddress& addr);

  // TODO add inform about event from tapi-sockets-tcp

  /* TCP states */
  const static StateMapPar new_states;
  typedef RState<RSocketTCP, ConnectionStateAxis, new_states> State;
  friend class RState<RSocketTCP, ConnectionStateAxis, new_states>;

  const static State closedState;
  const static State listenState;
  const static State syn_sentState;
  const static State establishedState;
  const static State closingState;
  const static State abortedState;
  const static State destroyedState;
protected:

  typedef Logger<RSocketTCP> log;
  
  /// It is set in the constructor by ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  /// how much wait closing the socket
  int close_wait_secs;

};
