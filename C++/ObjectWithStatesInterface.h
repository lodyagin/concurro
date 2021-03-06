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

#ifndef CONCURRO_OBJECTWITHSTATESINTERFACE_H_
#define CONCURRO_OBJECTWITHSTATESINTERFACE_H_

#include <typeinfo>
#include <type_traits>
#include "StateAxis.h"
#include "Event.h"

namespace curr {

class UniversalEvent;

//! @addtogroup states
//! @{

template<class Axis1, class Axis2> class RMixedAxis;
template<class Axis> class RState;
template<class Axis1, class Axis2> class RMixedEvent;

class AbstractObjectWithStates
{
  template<class Axis>
  friend class RObjectWithStates;

public:
  virtual ~AbstractObjectWithStates() {}

  //! the "update parent" callback on state changing in
  //! the `object' on the `state_ax' to `new_state'.
  //! @param ax is used for dispatching to particular
  //! RObjectWithStates class (used in RStateSplitter to
  //! monitor SplitAxis changes from DerivedAxis).
  virtual void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,
     AbstractObjectWithStates* object,
     const UniversalState& new_state) = 0;

#if 0
  //! Terminal state means 
  //! 1) no more state activity;
  //! 2) the object can be deleted (there are no more
  //! dependencies on it).
  virtual CompoundEvent is_terminal_state() const = 0;
#endif

  //! Return access to a current state atomic value.
  virtual std::atomic<uint32_t>& 
    current_state(const StateAxis&) = 0;

  //! Return constant access to a current state atomic
  //! value.
  virtual const std::atomic<uint32_t>& 
    current_state(const StateAxis&) const = 0;

#if 0
  /**
   * Exception: it is thrown by the default implementation
   * of state_is_broken.
   */
  class BrokenState : public curr::SException
  {
  public:
    BrokenState
      (const AbstractObjectWithStates& obj,
       const UniversalState& rb_transition_from,
       const UniversalState& rb_transition_to);
  };

  //! Notify the object when a compound operation needs to
  //! rollback the state change rb_transition_from ->
  //! rb_transition_to but unable to do so.
  virtual void state_is_broken
    (const UniversalState& rb_transition_from,
     const UniversalState& rb_transition_to) = 0;

#endif
};

class RObjectWithStatesBase;

/// An interface which should be implemented in each
/// state-aware class.
template<class Axis>
class ObjectWithStatesInterface
: public virtual AbstractObjectWithStates,
  public virtual ObjectWithLogging
{
  template<class Axis1, class Axis2> 
	 friend class RMixedAxis;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
public:
  typedef Axis axis;
  typedef RState<Axis> State;

  virtual ~ObjectWithStatesInterface() {}

  //! The default implementation returns just
  //! curr::type<*this>::name(). 
  virtual std::string object_name() const
  {
    return curr::type<decltype(*this)>::name();
  }
};

class AbstractObjectWithEvents
{
  template<class Axis1, class Axis2> 
	 friend class RStateSplitter;
public:
  virtual ~AbstractObjectWithEvents() {}

  //! Update events due to trans_id to
  virtual void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) = 0;

  //! Terminal state means 
  //! 1) no more state activity;
  //! 2) the object can be deleted (there are no more
  //! dependencies on it).
  virtual CompoundEvent is_terminal_state() const = 0;

#if 0
unable to have events from different axes in one class
protected:
  //! Query an event object by UniversalEvent. 
  virtual Event get_event(const UniversalEvent& ue) = 0;

  //! Query an event object by UniversalEvent. 
  virtual Event get_event
	 (const UniversalEvent& ue) const = 0;

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  virtual Event create_event
	 (const UniversalEvent&) const = 0;

#endif
};

template<class Axis>
class ObjectWithEventsInterface
: public virtual AbstractObjectWithEvents,
  public virtual ObjectWithLogging
{
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
  friend class RMixedAxis;
public:
  virtual ~ObjectWithEventsInterface() {}

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  virtual CompoundEvent create_event
     (const UniversalEvent&) const = 0;

  //! Update events due to trans_id to
  virtual void update_events
     (StateAxis& ax, 
      TransitionId trans_id, 
      uint32_t to) = 0;
};

// a new naming schema

namespace state {

template<class T>
using interface = ObjectWithStatesInterface<T>;

}

namespace event {

template<class T>
using interface = ObjectWithEventsInterface<T>;

template<class Axis>
class interface_with_states
  : public state::interface<Axis>,
    public interface<Axis>
{};

}

#if 0
struct state_finalizer_marker {};

template<
  template<class...> class Parent1,
  class... Ts
>
class repeater : public Parent1<Ts...>
{
public:
  using Parent1<Ts...>::Parent1;
};

template<
  template<template<class...> class, class...> class Parent2,
  template<class...> class Parent1,
  class... Ts
>
class state_finalizer : 
  public Parent2<Parent1, Ts...>,
  std::enable_if<
    !std::is_base_of
      <state_finalizer_marker, Parent2<Parent1, Ts...>>::value,
    state_finalizer_marker
  >::type
{
public:
  using Parent2<Parent1, Ts...>::Parent2;

  void state_changed(
    curr::StateAxis& ax,
    const curr::StateAxis& state_ax,
    curr::AbstractObjectWithStates* object,
    const curr::UniversalState& new_state) override
  {
    ax.state_changed(this, object, state_ax, new_state);
  }

  std::atomic<uint32_t>&
    current_state(const curr::StateAxis& ax) override
  {
    return ax.current_state(this);
  }

  const std::atomic<uint32_t>&
    current_state(const curr::StateAxis& ax) const override
  {
    return ax.current_state(this);
  } 
};
#endif

#define MULTIPLE_INHERITANCE_DEFAULT_STATE_MEMBERS        \
void state_changed(                                     \
  curr::StateAxis& ax, \
  const curr::StateAxis& state_ax,     \
  curr::AbstractObjectWithStates* object, \
  const curr::UniversalState& new_state)        \
{ \
  ax.state_changed(this, object, state_ax, new_state); \
} \
\
std::atomic<uint32_t>& \
current_state(const curr::StateAxis& ax) override \
{ \
  return ax.current_state(this); \
} \
\
const std::atomic<uint32_t>& \
current_state(const curr::StateAxis& ax) const override \
{ \
  return ax.current_state(this); \
} 

#define MULTIPLE_INHERITANCE_DEFAULT_EVENT_MEMBERS  \
\
void update_events \
(curr::StateAxis& ax, \
 curr::TransitionId trans_id, \
 uint32_t to) override \
{ \
  ax.update_events(this, trans_id, to); \
}

#define MULTIPLE_INHERITANCE_DEFAULT_MEMBERS  \
MULTIPLE_INHERITANCE_DEFAULT_STATE_MEMBERS    \
MULTIPLE_INHERITANCE_DEFAULT_EVENT_MEMBERS

//! @}

}
#endif
