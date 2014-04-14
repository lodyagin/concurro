/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHSTATES_H_
#define CONCURRO_ROBJECTWITHSTATES_H_

#include <atomic>
#include "SCommon.h"
#include "Event.h"
#include "ObjectWithStatesInterface.h"
#include "StateMap.h"
#include "StateAxis.h"
#include "RState.h"

namespace curr {

//! @addtogroup exceptions
//! @{

class REventIsUnregistered : public SException
{
public:
  REventIsUnregistered(const UniversalEvent& ue)
    : SException(SFORMAT("The event [" << ue 
                         << "] is unregistered")),
      event(ue) 
  {}

  const UniversalEvent event;
};

//! @}

//! @addtogroup states
//! @{

//TODO allow only delegates have subscribers list
template<size_t max_subscribers = 0>
class RObjectWithStatesBase
  : public virtual AbstractObjectWithStates
{
public:
  RObjectWithStatesBase() : n_subs(0) {}

  RObjectWithStatesBase(RObjectWithStatesBase&) = delete;

  RObjectWithStatesBase& operator=(
    RObjectWithStatesBase&
  ) = delete;

  virtual ~RObjectWithStatesBase();

  void register_subscriber
    (AbstractObjectWithEvents*, StateAxis*);

  //! An "update parent" callback on state changing in
  //! the `object'.
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;

protected:
  struct Subscriber 
  {
    std::atomic<bool> ready;
    AbstractObjectWithEvents* object;
    StateAxis* axis;
    CompoundEvent terminal;

    Subscriber() : ready(false) {}
  };

  std::array<Subscriber, max_subscribers> subscribers;
  std::atomic<size_t> n_subs;
};

template<>
class RObjectWithStatesBase<0>
  : public virtual AbstractObjectWithStates
{
public:
  virtual ~RObjectWithStatesBase() {}

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {}
};

//! It can be used as a parent of an object which
//! introduces new state axis.
template<
  class Axis, 
  class FinalAxis,
  size_t max_subscribers
>
class RObjectWithStates 
  : public virtual ObjectWithStatesInterface<Axis>,
    public RObjectWithStatesBase<max_subscribers>
{
public:
  typedef typename ObjectWithStatesInterface<FinalAxis>
    ::State State;

  typedef AbstractMethodCallWrapper<
    RObjectWithStates<Axis, FinalAxis, max_subscribers>, 
    AbstractObjectWithStates*,
    const StateAxis&,
    const UniversalState&
  > AMembWrap;

  template<class AppObj>
  using MembWrap = MethodCallWrapper<
    AppObj, 
    RObjectWithStates<Axis, FinalAxis, max_subscribers>, 
    AbstractObjectWithStates*,
    const StateAxis&,
    const UniversalState&
  >;

  RObjectWithStates
    (const State& initial_state, AMembWrap* amcw = nullptr)
    : currentState(initial_state), mcw(amcw)
  {}

  template<class AppObj>
    static typename RObjectWithStates
      <Axis, FinalAxis, max_subscribers>
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
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;

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

template<class Axis, class FinalAxis>
class RObjectWithStates<Axis, FinalAxis, 0>
  : public virtual ObjectWithStatesInterface<Axis>,
    public RObjectWithStatesBase<0>
{
public:
  typedef void AMembWrap;

  typedef typename ObjectWithStatesInterface<FinalAxis>
    ::State State;

  RObjectWithStates(const State& initial_state)
    : currentState(initial_state)
  {}

  RObjectWithStates(const RObjectWithStates&) = delete;

  RObjectWithStates& operator=(
    const RObjectWithStates&
  ) = delete;

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {}

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
};

DECLARE_AXIS(MoveableAxis, StateAxis);

template<class FinalAxis>
class RObjectWithStates<MoveableAxis, FinalAxis, 0>
  : public virtual ObjectWithStatesInterface<MoveableAxis>,
    public RObjectWithStatesBase<0>
{
public:
  static_assert(is_ancestor<MoveableAxis, FinalAxis>(), 
     "FinalAxis must be derived from MoveableAxis.");

  typedef void AMembWrap;

  //! @cond
  DECLARE_STATES(MoveableAxis, State);
  DECLARE_STATE_CONST(State, moving_from);
  DECLARE_STATE_CONST(State, moving_to);
  DECLARE_STATE_CONST(State, copying_from);
  DECLARE_STATE_CONST(State, moved_from);
  //! @endcond

  RObjectWithStates(
    const RState<FinalAxis>& initial_state
  )
    : currentState(initial_state)
  {}

  RObjectWithStates(const RObjectWithStates& o) = default;
  RObjectWithStates(RObjectWithStates&& o);

  RObjectWithStates& operator=(const RObjectWithStates& o)
    = default;
  RObjectWithStates& operator=(RObjectWithStates&& o);

//  void swap(RObjectWithStates&);

#if 0
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;
#endif

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
};

class UniversalEvent;

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

template<
  class Axis, 
  class FinalAxis = Axis,
  size_t max_subscribers = 0
>
class RObjectWithEvents
  : public RObjectWithStates
      <Axis, FinalAxis, max_subscribers>,
    public virtual ObjectWithEventsInterface<Axis>
{
  template<class Axis1, class Axis2> 
    friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
    friend class RMixedAxis;
  template<class Axis1, class Axis2, size_t, size_t> 
    friend class RStateSplitter;
public:
  typedef RObjectWithStates
    <Axis, FinalAxis, max_subscribers> Parent;
  typedef typename Parent::State State;
  typedef typename Parent::AMembWrap AMembWrap;

  using Parent::Parent;

#if 0
  RObjectWithEvents(
    const State& initial_state, 
    AMembWrap* mcw = nullptr
  );

  template<class AppObj>
  static typename Parent::template MembWrap<AppObj>*
  state_hook(
    void (AppObj::*memb)(
      AbstractObjectWithStates*,
      const StateAxis&,
      const UniversalState&
    )
  )
  {
    return Parent::state_hook(memb);
  }
#endif

  //! Update events due to trans_id to
  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to);

protected:
  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  CompoundEvent create_event
    (const UniversalEvent&) const override;

  typedef std::map<uint32_t, Event> EventMap;

  //! It maps a local event id to an Event object
  mutable EventMap events;
};

#if 0
template<
  class Axis, 
  class FinalAxis = Axis,
  size_t max_subscribers = 0
>
class RObjectWithEvents<Axis, FinalAxis, 0>
  : public RObjectWithStates<Axis, FinalAxis, 0>,
    public virtual ObjectWithEventsInterface<Axis>
{
  template<class Axis1, class Axis2> 
    friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
    friend class RMixedAxis;
  template<class Axis1, class Axis2, size_t, size_t> 
    friend class RStateSplitter;
public:
  typedef RObjectWithStates<Axis, FinalAxis, 0> Parent;
  typedef typename Parent::State State;
  typedef typename Parent::AMembWrap AMembWrap;

  using Parent::Parent;

  //! Update events due to trans_id to
  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to);

protected:
  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  CompoundEvent create_event
    (const UniversalEvent&) const override;

  typedef std::map<uint32_t, Event> EventMap;

  //! It maps a local event id to an Event object
  mutable EventMap events;
};
#endif

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

template<
  class DerivedAxis, 
  class SplitAxis,
  size_t maxs = 0,
  size_t maxs_delegate = 0>
class RStateSplitter 
: public RObjectWithEvents<DerivedAxis, DerivedAxis, maxs>,
  public virtual ObjectWithEventsInterface<SplitAxis>,
  public virtual ObjectWithStatesInterface<SplitAxis>
{
public:
  typedef typename ObjectWithStatesInterface<DerivedAxis>
    ::State State;
  typedef RObjectWithEvents<DerivedAxis, DerivedAxis, maxs>
    Parent;
  typedef typename Parent::AMembWrap AMembWrap;
  typedef RObjectWithEvents
    <SplitAxis, SplitAxis, maxs_delegate> Delegate;

  //! Construct a delegator to delegate all states not
  //! covered by DerivedAxis.
  RStateSplitter(
    RObjectWithEvents
      <SplitAxis, SplitAxis, maxs_delegate>* a_delegate,
    const State& initial_state,
    AMembWrap* mcw = nullptr
  );

  RStateSplitter(const RStateSplitter&) = delete;

  RStateSplitter* operator=
    (const RStateSplitter&) = delete;

#if 0
  template<class AppObj>
  static typename 
    RObjectWithStates<DerivedAxis, DerivedAxis, maxs>
  ::template MembWrap<AppObj>*
  state_hook(void (AppObj::*memb) 
             (AbstractObjectWithStates*,
              const StateAxis&,
              const UniversalState& new_state))
  {
    return RObjectWithEvents<DerivedAxis>
      ::state_hook(memb);
  }
#endif

  virtual void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,    
     AbstractObjectWithStates* object,
     const UniversalState& new_state) = 0;

protected:
  //! The 2nd stage init. virtual calls / callbacks to
  //! this are allowed only after the call.
  void init() const;

  std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) override
  {
    assert(is_same_axis<DerivedAxis>(ax)
           || is_same_axis<SplitAxis>(ax));
    return Parent::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  {
    assert(is_same_axis<DerivedAxis>(ax)
           || is_same_axis<SplitAxis>(ax));
    return Parent::current_state(ax);
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

  Delegate* delegate;
  const uint16_t split_state_id;
  const TransitionId split_transition_id;
  mutable bool inited;
};

#define RSTATESPLITTER_DEFAULT_MEMBERS(DerivedAxis, SplitAxis) \
void state_changed \
  (StateAxis& ax,  \
   const StateAxis& state_ax, \
   AbstractObjectWithStates* object, \
   const curr::UniversalState& new_state) override \
{ \
  ax.state_changed(this, object, state_ax, new_state);  \
} \
\
std::atomic<uint32_t>& current_state(const StateAxis& ax) override \
{ \
  return RStateSplitter<DerivedAxis, SplitAxis>::current_state(ax); \
} \
\
const std::atomic<uint32_t>& \
current_state(const StateAxis& ax) const override \
{ \
  return RStateSplitter<DerivedAxis, SplitAxis>::current_state(ax); \
} \
\
CompoundEvent create_event \
(const UniversalEvent& ue) const override \
{ \
  return RStateSplitter<DerivedAxis, SplitAxis>::create_event(ue); \
} \
\
void update_events \
  (StateAxis& ax, TransitionId trans_id, uint32_t to) override \
{ \
  ax.update_events(this, trans_id, to); \
}

//! @}
  
}
#endif

