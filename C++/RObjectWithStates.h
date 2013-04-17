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
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif

class REventIsUnregistered : public SException
{
public:
  REventIsUnregistered(const UniversalEvent& ue)
	 : SException(SFORMAT("The event [" << ue 
								 << "] is unregistered")),
	 event(ue) {}

  const UniversalEvent event;
};

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

  const std::atomic<uint32_t>& current_state() const
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
  Event get_event(const UniversalEvent& ue)
  {
	 return get_event_impl(ue);
  }

  //! Query an event object by UniversalEvent. 
  const Event get_event(const UniversalEvent& ue) const
  {
	 return get_event_impl(ue);
  }

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  Event create_event(const UniversalEvent&) const;

  //! Update events due to trans_id to
  void update_events
	 (TransitionId trans_id, uint32_t to);

  typedef std::map<uint32_t, Event> EventMap;

  mutable EventMap events;

private:
  //! A common implementation for both get_event
  Event get_event_impl(const UniversalEvent&) const;
};

#endif

