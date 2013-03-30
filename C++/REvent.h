// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_H
#define CONCURRO_REVENT_H

#include "StateMap.h"
#include "RObjectWithStates.h"
#include "Event.h"

class UniversalEvent
{
public:
  //const StateMap* state_map;
  TransitionId    transition_id;
protected:
  UniversalEvent(TransitionId trans_id)
	 : transition_id(trans_id) {}
};

template<
//class Object, 
  class Axis
  >
class REvent 
: public Axis, 
  virtual protected UniversalEvent
  //public SAutoSingleton<REvent<Object, Axis>>
{
public:
  //! Create a from->to event for Object in Axis.
  REvent(const char* from, const char* to);
  bool wait(RObjectWithEvents<Axis>&, int time 
				= std::numeric_limits<uint64_t>::max());  
};

//! Create an event of moving Obj from `before' state to
//! `after' state.
const UniversalEvent& operator / 
  (const UniversalState& before, 
	const UniversalState& after);

//! Change a state according to an event
const UniversalState& operator + 
  (const UniversalState& state, 
	const UniversalEvent& event);

//size_t waitMultiple( HANDLE *, size_t count );

// include shutdown event also
//size_t waitMultipleSD( HANDLE *, size_t count );

#include "REvent.hpp"

#endif 
