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
#include <cstdatomic>

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
	 : currentState(initial_state) {}
  RObjectWithStates(const RObjectWithStates&);

  virtual ~RObjectWithStates() {}

  RObjectWithStates& operator=(const RObjectWithStates&);

protected:

  std::atomic<uint32_t>& current_state()
  {
	 return currentState;
  }

  std::atomic<uint32_t> currentState;
};

class UniversalEvent;

template<class Axis>
class REvent;

template<class Axis>
class RObjectWithEvents
  : public RObjectWithStates<Axis>
{
  friend class REvent<Axis>;
  friend class RState<Axis>;
  friend class RAxis<Axis>;
public:
  typedef RObjectWithStates<Axis> Parent;
  typedef typename Parent::State State;

  RObjectWithEvents
	 (const typename RObjectWithStates<Axis>::State& 
	    initial_state)
	 : Parent(initial_state)
  { }

protected:
  //! Query an event object by UniversalEvent. 
  //! Also create new event object if it doesn't exists.
  Event* get_event(const UniversalEvent&);

  //! Update events due to trans_id to
  void update_events
	 (TransitionId trans_id, uint32_t to);

  typedef std::map<TransitionId, Event*> EventMap;

  EventMap events;
};

#endif

