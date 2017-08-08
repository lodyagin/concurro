/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

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

#include "SCommon.h"
//#include "Event.h"
#include "ObjectWithStatesInterface.h"
#include <atomic>

namespace curr {

//! @addtogroup exceptions
//! @{

class REventIsUnregistered : public virtual std::exception
{
/*public:
REventIsUnregistered(const UniversalEvent& ue)
  : SException(SFORMAT("The event [" << ue 
                       << "] is unregistered")),
    event(ue) {}

  const UniversalEvent event;
*/
};

//! @}

//! @addtogroup states
//! @{

//TODO allow only delegates have subscribers list
class RObjectWithStatesBase
: public virtual AbstractObjectWithStates
{
public:
  RObjectWithStatesBase();
  RObjectWithStatesBase(RObjectWithStatesBase&&);
  virtual ~RObjectWithStatesBase();

  RObjectWithStatesBase& operator=
    (RObjectWithStatesBase&&);

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
  // No more changes in subscribers list
  //std::atomic<bool> is_frozen;
  //std::atomic<bool> is_changing;
  RMutex subscribe_mt
    { "RObjectWithStatesBase::subscribe_mt" };

  typedef std::pair<AbstractObjectWithEvents*, StateAxis*>
    Subscriber;

  //! Registered subscribers
  std::set<Subscriber> subscribers;
#ifdef USE_EVENTS
  //! All subscribers terminal states.
  std::set<CompoundEvent> subscribers_terminals;
#endif
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
  RObjectWithStates(RObjectWithStates&&);

  virtual ~RObjectWithStates() {}

  RObjectWithStates& operator=(const RObjectWithStates&);
  RObjectWithStates& operator=(RObjectWithStates&&);

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

#ifdef USE_EVENTS
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

  using states_interface = 
    curr::event::interface_with_states<Axis>;

  RObjectWithEvents
    (const typename RObjectWithStates<Axis>::State& 
     initial_state,
     AMembWrap* mcw = nullptr);

  RObjectWithEvents(RObjectWithEvents&&);

  RObjectWithEvents& operator= (RObjectWithEvents&&);

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

template<class DerivedAxis, class SplitAxis>
class RStateSplitter 
#ifdef USE_EVENTS
: public RObjectWithEvents<DerivedAxis>,
  public virtual ObjectWithEventsInterface<SplitAxis>,
  public virtual ObjectWithStatesInterface<SplitAxis>
#else
: public RObjectWithStates<DerivedAxis>,
  public virtual ObjectWithStatesInterface<SplitAxis>
#endif
{
public:
  typedef typename ObjectWithStatesInterface<DerivedAxis>
    ::State State;
#ifdef USE_EVENTS
  typedef typename RObjectWithEvents<DerivedAxis>
    ::AMembWrap AMembWrap;
#else
  typedef typename RObjectWithStates<DerivedAxis>
    ::AMembWrap AMembWrap;
#endif

  //! Construct a delegator to delegate all states not
  //! covered by DerivedAxis.
  RStateSplitter
    (
#ifdef USE_EVENTS
     RObjectWithEvents<SplitAxis>* a_delegate,
#else
     RObjectWithStates<SplitAxis>* a_delegate,
#endif
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
#ifdef USE_EVENTS
    return RObjectWithEvents<DerivedAxis>
#else
    return RObjectWithStates<DerivedAxis>
#endif
      ::state_hook(memb);
  }

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
#ifdef USE_EVENTS
    return RObjectWithEvents<DerivedAxis>
#else
    return RObjectWithStates<DerivedAxis>
#endif
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  {
    assert(is_same_axis<DerivedAxis>(ax)
           || is_same_axis<SplitAxis>(ax));
#ifdef USE_EVENTS
    return RObjectWithEvents<DerivedAxis>
#else
    return RObjectWithStates<DerivedAxis>
#endif
      ::current_state(ax);
  }

#ifdef USE_EVENTS
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
#else
  RObjectWithStates<SplitAxis>* delegate;
#endif
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

