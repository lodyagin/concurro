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

#if 0
class RStateChangeSubscriber
{
public:
  //! the "update parent" callback on state changing in
  //! the `object'.
  virtual void state_changed
	 (AbstractObjectWithStates* object) = 0;
  //! Terminal state means 
  //! 1) no more state activity;
  //! 2) the object can be deleted (there are no more
  //! dependencies on it).
  virtual CompoundEvent is_terminal_state() const = 0;
};
#endif

class RObjectWithStatesBase
{
public:
  virtual ~RObjectWithStatesBase();
  //void register_subscriber(RStateChangeSubscriber*);
  void register_subscriber(RObjectWithStatesBase*);
  //! the "update parent" callback on state changing in
  //! the `object'.
  void state_changed(AbstractObjectWithStates* object);
  //! Terminal state means 
  //! 1) no more state activity;
  //! 2) the object can be deleted (there are no more
  //! dependencies on it).
  virtual CompoundEvent is_terminal_state() const = 0;
protected:
  //! No more changes in subscribers list
  std::atomic<bool> is_frozen;
  std::atomic<bool> is_changing;
  //! Registered subscribers
  //std::set<RStateChangeSubscriber*> subscribers;
  std::set<RObjectWithStatesBase*> subscribers;
  //! All subscribers terminal states.
  std::set<CompoundEvent> subscribers_terminals;
};

//! It can be used as a parent of an object which
//! introduces new state axis.
template<class Axis>
class RObjectWithStates 
: public virtual ObjectWithStatesInterface<Axis>,
  public RObjectWithStatesBase
{
public:
  typedef typename ObjectWithStatesInterface<Axis>
	 ::State State;
  
  RObjectWithStates(const State& initial_state)
	 : currentState(initial_state) {}
  RObjectWithStates(const RObjectWithStates&);

  virtual ~RObjectWithStates() {}

  RObjectWithStates& operator=(const RObjectWithStates&);

  void state_changed(AbstractObjectWithStates* object) 
	 override
  {
	 RObjectWithStatesBase::state_changed(object);
  }

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
  public virtual ObjectWithEventsInterface<Axis>
{
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
	 friend class RMixedAxis;
  template<class Axis1, class Axis2> 
	 friend class RStateSplitter;
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
  Event get_event
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

template <class Axis>
class RStateDelegator
: public virtual ObjectWithStatesInterface<Axis>
{
public:
  RStateDelegator(RObjectWithStates<Axis>* a_delegate)
	 : delegate(a_delegate) {}

  std::atomic<uint32_t>& current_state() override
  {
	 return delegate->current_state();
  }

  const std::atomic<uint32_t>& current_state() const 
	 override
  {
	 return delegate->current_state();
  }

protected:
  RObjectWithStates<Axis>* delegate;
};

//! It can maintain two states or delegate a "parent"
//! part of states and events to another class ("parent"
//! states are states from DerivedAxis). SplitAxis changes will arrive
//! to us by state_change() notification. DerivedAxis
//! changes are local and not propagated to a delegate yet
//! (TODO).
//! <NB> Both delegate and splitter maintain different and
//! not intersected sets of events. It is also possible to have
//! 2 events set on different axes on the same time.

template<class DerivedAxis, class SplitAxis>
class RStateSplitter 
  : public RObjectWithEvents<DerivedAxis>,
  public RStateDelegator<SplitAxis>,
  public virtual ObjectWithEventsInterface<SplitAxis>
{
public:
  typedef typename ObjectWithStatesInterface<DerivedAxis>
	 ::State State;
  
  //! Construct a delegator to delegate all states not
  //! covered by DerivedAxis.
  RStateSplitter
	 (RObjectWithEvents<SplitAxis>* a_delegate,
	  const State& initial_state);

  RStateSplitter(const RStateSplitter&) = delete;

  RStateSplitter* operator=
	 (const RStateSplitter&) = delete;

protected:
  Event get_event(const UniversalEvent& ue) override;

  Event get_event(const UniversalEvent& ue) const override;

  Event create_event
	 (const UniversalEvent&) const override;

  void update_events
	 (TransitionId trans_id, uint32_t to) override;

  //! Select this or delegate depending on the event.
  //! \return true if it is from DerivedAxis
  bool is_this_event_store
	 (const UniversalEvent& ue) const;

  RObjectWithEvents<SplitAxis>* delegate;
  //uint16_t not_delegate;
  const uint16_t split_state_id;
  const TransitionId split_transition_id;
};

#endif

