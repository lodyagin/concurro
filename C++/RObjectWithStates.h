// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_H_
#define CONCURRO_ROBJECTWITHSTATES_H_

#include "SCommon.h"
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

class RObjectWithStatesBase
: public virtual AbstractObjectWithStates
{
public:
  RObjectWithStatesBase();
  virtual ~RObjectWithStatesBase();

  void register_subscriber
    (RObjectWithStatesBase*, StateAxis*);

  //! An "update parent" callback on state changing in
  //! the `object'.
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override;

protected:
  //! No more changes in subscribers list
  std::atomic<bool> is_frozen;
  std::atomic<bool> is_changing;

  typedef std::pair<RObjectWithStatesBase*, StateAxis*>
    Subscriber;

  //! Registered subscribers
  std::set<Subscriber> subscribers;
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

  typedef AbstractMethodCallWrapper<
    RObjectWithStates<Axis>, 
    AbstractObjectWithStates*,
    const StateAxis&,
    const UniversalState&
  > AMembWrap;

  template<class AppObj>
  using MembWrap = MethodCallWrapper<
    AppObj, RObjectWithStates<Axis>, 
    AbstractObjectWithStates*,
    const StateAxis&,
    const UniversalState&
  >;

  RObjectWithStates
    (const State& initial_state, AMembWrap* amcw = nullptr)
    : currentState(initial_state), mcw(amcw)
  {}

  RObjectWithStates(const RObjectWithStates&);

  virtual ~RObjectWithStates() {}

  RObjectWithStates& operator=(const RObjectWithStates&);

  template<class AppObj>
    static typename RObjectWithStates<Axis>
    ::template MembWrap<AppObj>*
    state_hook(void (AppObj::*memb) 
               (AbstractObjectWithStates*, 
                const StateAxis&,
                const UniversalState&))
  {
    return new MembWrap<AppObj>(memb);
  }

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) override;

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  {
    return currentState;
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  {
    return currentState;
  }

protected:
  std::atomic<uint32_t> currentState;
  std::shared_ptr<AMembWrap> mcw;
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
  typedef typename RObjectWithStates<Axis>::AMembWrap
    AMembWrap;

  RObjectWithEvents
    (const typename RObjectWithStates<Axis>::State& 
     initial_state,
     AMembWrap* mcw = nullptr)
    : Parent(initial_state, mcw)
  { }

  template<class AppObj>
    static typename RObjectWithStates<Axis>
    ::template MembWrap<AppObj>*
    state_hook(void (AppObj::*memb) 
               (AbstractObjectWithStates*,
                const StateAxis&,
                const UniversalState&))
  {
    return RObjectWithStates<Axis>::state_hook(memb);
  }

  //! Update events due to trans_id to
  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to);

protected:
#if 0
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
#endif

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  CompoundEvent create_event
    (const UniversalEvent&) const override;

  typedef std::map<uint32_t, Event> EventMap;

  //! It maps a local event id to an Event object
  mutable EventMap events;

private:
#if 0
  //! A common implementation for both get_event
  Event get_event_impl(const UniversalEvent&) const;
#endif
};

//! It can maintain two states or delegate a "parent"
//! part of states and events to another class ("parent"
//! states are states from DerivedAxis). SplitAxis changes
//! will arrive
//! to us by state_change() notification. DerivedAxis
//! changes are local and not propagated to a delegate yet
//! (TODO).
//! <NB> Both delegate and splitter maintain different and
//! not intersected sets of events. It is also possible to
//! have 2 events set on different axes on the same time.

template<class DerivedAxis, class SplitAxis>
class RStateSplitter 
: public RObjectWithEvents<DerivedAxis>,
  public virtual ObjectWithEventsInterface<SplitAxis>,
  public virtual ObjectWithStatesInterface<SplitAxis>
{
public:
  typedef typename ObjectWithStatesInterface<DerivedAxis>
    ::State State;
  typedef typename RObjectWithEvents<DerivedAxis>
    ::AMembWrap AMembWrap;

  //! Construct a delegator to delegate all states not
  //! covered by DerivedAxis.
  RStateSplitter
    (RObjectWithEvents<SplitAxis>* a_delegate,
     const State& initial_state,
     AMembWrap* mcw = nullptr);

  RStateSplitter(const RStateSplitter&) = delete;

  RStateSplitter* operator=
    (const RStateSplitter&) = delete;

  template<class AppObj>
    static typename RObjectWithStates<DerivedAxis>
    ::template MembWrap<AppObj>*
    state_hook(void (AppObj::*memb) 
               (AbstractObjectWithStates*,
                const StateAxis&,
                const UniversalState& new_state))
  {
    return RObjectWithEvents<DerivedAxis>
      ::state_hook(memb);
  }

  virtual void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object) = 0;

  /*StateAxis& get_axis() const override
    {
    return DerivedAxis::self();
    }*/

protected:
  //! The 2nd stage init.
  void init() const;

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  {
    assert(is_same_axis<DerivedAxis>(ax)
           || is_same_axis<SplitAxis>(ax));
    init();
    return RObjectWithEvents<DerivedAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  {
    assert(is_same_axis<DerivedAxis>(ax)
           || is_same_axis<SplitAxis>(ax));
    init();
    return RObjectWithEvents<DerivedAxis>
      ::current_state(ax);
  }

  CompoundEvent create_event
    (const UniversalEvent&) const override;

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override;

  //! Select this or delegate depending on the event.
  //! \return true if it is from DerivedAxis
  bool is_this_event_store
    (const UniversalEvent& ue) const;

  RObjectWithEvents<SplitAxis>* delegate;
  const uint16_t split_state_id;
  const TransitionId split_transition_id;
  mutable bool inited;
};

#endif

