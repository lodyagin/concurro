#ifndef CONCURRO_ABSTRACTCONNECTION_H_
#define CONCURRO_ABSTRACTCONNECTION_H_

#include "StateMap.h"
#include "RObjectWithStates.h"
#include "SNotCopyable.h"
#include "REvent.h"

class ConnectionStateAxis : public StateAxis {};

class AbstractConnection
 : public SNotCopyable, 
	//   public RObjectWithStates<ConnectionStateAxis>
   public ObjectWithStatesInterface<ConnectionStateAxis>
{
public:

  struct Par 
  {
	 REvent* connectionTerminated;
    Par() : connectionTerminated(0) {}
	 virtual ~Par() {}
  };

  AbstractConnection
	 (const std::string& id, REvent* terminated)
	 : universal_object_id(id),
	   connectionTerminated(terminated) {}
  virtual ~AbstractConnection() {}

  virtual void ask_open () = 0;
  virtual void ask_close () = 0;

  const static StateMapPar new_states;

  typedef RState
	 <AbstractConnection, ConnectionStateAxis, new_states> 
	 State;

  friend class RState
	 <AbstractConnection, ConnectionStateAxis, new_states>;

  const static State closedState;
  const static State establishedState;
  const static State destroyedState;

  const std::string universal_object_id;

  std::string universal_id() const
  {
	 return universal_object_id;
  }

protected:

  REvent* connectionTerminated;
};

#endif
