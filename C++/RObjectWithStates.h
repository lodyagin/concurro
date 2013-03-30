// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_H_
#define CONCURRO_ROBJECTWITHSTATES_H_

#include "Event.h"
#include "ObjectWithStatesInterface.h"

//! It can be used as a parent of an object which
//! introduces new state axis.
template<class Axis>
class RObjectWithStates 
  : public ObjectWithStatesInterface<Axis>
{
public:
  typedef typename ObjectWithStatesInterface<Axis>
	 ::State State;

  RObjectWithStates(const State& initial_state)
	 : currentState(initial_state) 
  {}

  virtual ~RObjectWithStates() {}

  /// set state to the current state of the object
  virtual void state(State& state) const
  {
	 state.set_by_universal (currentState);
  }

  virtual bool state_is(const State& state) const
  {
	 return currentState == (UniversalState) state;
  }

protected:

  virtual void set_state_internal (const State& state);

  UniversalState currentState;
};

template<class Axis>
class REvent;

template<class Axis>
class RObjectWithEvents
  : public RObjectWithStates<Axis>
{
  friend class REvent<Axis>;
public:
  RObjectWithEvents
	 (const typename RObjectWithStates<Axis>::State& 
	    initial_state)
	 : RObjectWithStates<Axis>(initial_state)
  { }

protected:
  //! Query an event object by transition id. Also create
  //! one if it doesn't exists.
  Event* get_event(TransitionId);

  typedef std::map<TransitionId, Event*> EventMap;

  EventMap events;
};

#include "RObjectWithStates.hpp"

#endif

