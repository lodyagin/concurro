/*
It is a connection created by the socket.
*/

#ifndef CONCURRO_RCONNECTION_H_
#define CONCURRO_RCONNECTION_H_

#include "RThread.h"
#include "Logging.h"
#include "StateMap.h"

class ConnectionStateAxis : public StateAxis {};

template<class Thread, class Socket>
class RConnection : public Thread
{
public:

  const std::string universal_object_id;
  
  RConnectedSocket* get_socket ()
  {
    assert (socket);
    return socket;
  }

  ~RConnection ()
  {
    delete socket;
  }

  /* Connection states */
  const static State2Idx newStates[];
  const static StateTransition newTrans[];
  typedef RState
    <RConnection, ConnectionStateAxis, newStates, newTrans> State;
  friend class RState
    <RConnection, ConnectionStateAxis, newStates, newTrans>;

  const static State closedState;
  const static State establishedState;

  void state (State& state) const
  {
	 state = currentState;
  }

protected:
  typedef Logger<RConnection> log;

  // Usually it is called by ConnectionFactory
  // RConnection takes the socket ownership
  // and will destroy it.
  RConnection 
    (void* repo, 
     RConnectedSocket* cs,
     const std::string& objId,
     const typename Thread::ConstrPar& par,
     REvent* connectionTerminated
     )
   : Thread (connectionTerminated, par),
     socket (cs), 
     repository (repo), 
     universal_object_id (objId)
  {
    assert (repo);
    assert (socket);
  }

  void set_state_internal (const State& state)
  {
    currentState = state;
  }

  RConnectedSocket* socket;
  void* repository;

private:
  State currentState;
};

#endif
