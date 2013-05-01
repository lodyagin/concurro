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

  std::atomic<uint32_t>& current_state() override
  {
	 return currentState;
  }

  const std::atomic<uint32_t>& 
	 current_state() const override
  {
	 return currentState;
  }

  std::atomic<uint32_t> currentState;
};

class UniversalEvent;

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

template<class Axis>
class RObjectWithEvents
: public RObjectWithStates<Axis>,
  public ObjectWithEventsInterface<Axis>
{
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
	 friend class RMixedAxis;
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
  Event get_event(const UniversalEvent& ue) override
  {
	 return get_event_impl(ue);
  }

  //! Query an event object by UniversalEvent. 
  const Event get_event
	 (const UniversalEvent& ue) const override
  {
	 return get_event_impl(ue);
  }

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  Event create_event
	 (const UniversalEvent&) const override;

  //! Update events due to trans_id to
  void update_events
	 (TransitionId trans_id, uint32_t to);

  typedef std::map<uint32_t, Event> EventMap;

  //! It maps a local event id to an Event object
  mutable EventMap events;

private:
  //! A common implementation for both get_event
  Event get_event_impl(const UniversalEvent&) const;
};

//! It delegates a part of states and eventsto another
//! class.
template<class Axis, class DerivedAxis>
class RStatesDelegator 
  : public ObjectWithStatesInterface<Axis>,
    public ObjectWithEventsInterface<Axis>
{
public:
  typedef typename ObjectWithStatesInterface<Axis>
	 ::State State;
  
  //! Construct a delegator to delegate all states not
  //! covered by DerivedAxis.
  RStatesDelegator
	 (ObjectWithStatesInterface<Axis>* a_delegate,
	  //const RState<DerivedAxis>& not_delegate_from,
	  const State& initial_state);
  RStatesDelegator(const RStatesDelegator&) = delete;

  RStatesDelegator* operator=
	 (const RStatesDelegator&) = delete;

  std::atomic<uint32_t>& current_state() override;

  const std::atomic<uint32_t>&
	 current_state() const override;

  Event get_event(const UniversalEvent& ue) override;

  const Event get_event
	 (const UniversalEvent& ue) const override;

  Event create_event
	 (const UniversalEvent&) const override;

  void update_events
	 (TransitionId trans_id, uint32_t to) override;

protected:
  ObjectWithStatesInterface<Axis>* delegate;
  uint16_t not_delegate;
};

#endif

