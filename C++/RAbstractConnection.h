#ifndef CONCURRO_RABSTRACT_CONNECTION_H_
#define CONCURRO_RABSTRACT_CONNECTION_H_

#include "StateMap.h"
#include "RObjectWithStates.h"
#include "SNotCopyable.h"

class ConnectionStateAxis : public StateAxis {};

class RAbstractConnection
: public SNotCopyable, public RObjectWithStates<ConnectionStateAxis>
{
public:
  RAbstractConnection() 
	 : RObjectWithStates<ConnectionStateAxis>
	 (dynamic_cast<const ConnectionStateAxis&>(closedState)) 
  {}

  virtual ~RAbstractConnection() {}

  virtual void close () = 0;

  const static StateMapPar new_states;

  typedef RState<RAbstractConnection, ConnectionStateAxis, new_states> State;
  friend class RState<RAbstractConnection, ConnectionStateAxis, new_states>;

  const static State closedState;
  const static State establishedState;
  const static State destroyedState;
};

#endif
