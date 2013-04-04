// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_ABSTRACTCONNECTION_H_
#define CONCURRO_ABSTRACTCONNECTION_H_

#include "StateMap.h"
#include "RState.h"
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
	 Event* connectionTerminated;
    Par() : connectionTerminated(0) {}
	 virtual ~Par() {}
  };

  AbstractConnection
	 (const std::string& id, Event* terminated)
	 : universal_object_id(id),
	   connectionTerminated(terminated) {}
  virtual ~AbstractConnection() {}

  virtual void ask_open () = 0;
  virtual void ask_close () = 0;

  DECLARE_STATES(ConnectionStateAxis, State);
  DECLARE_STATE_CONST(State, closedState);
  DECLARE_STATE_CONST(State, establishedState);
  DECLARE_STATE_CONST(State, destroyedState);

  const std::string universal_object_id;

  std::string universal_id() const
  {
	 return universal_object_id;
  }

protected:

  Event* connectionTerminated;
};

#endif
