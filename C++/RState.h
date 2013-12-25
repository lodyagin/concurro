/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
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

#ifndef CONCURRO_RSTATE_H_
#define CONCURRO_RSTATE_H_

#include <type_traits>
#include <mutex>

#include "types/meta.h"
#include "StateMap.h"
#include "ObjectWithStatesInterface.h"
#include "StateAxis.h"

namespace curr {

typedef int16_t StateMapId;

/**
 * @defgroup states
 *
 *  A state is an element from some set named "state
 *  space". A state defines a point in that space where a
 *  particular object resides (and this point is not
 *  constant for the class of objects). Thus, we can treat
 *  state both as a source and a feature of an object
 *  mutability.
 *
 *  A state space is not singular but each object class
 *  can have its own state spaces (e.g., the object class
 *  "a door with solid painted sides" has 3 spaces:
 *  (opened, closed) and 2 color spaces (for each
 *  side) but they are not universal state spaces because
 *  not each object class uses it).
 *
 *  An essential property of a state is a mutability and
 *  the mutability has its own rules. We always should
 *  control state transitions. Also, we can extend state
 *  and transitions sets in descendants (see ... and ...).
 *
 *  And we always use a notion of state space axis because the
 *  space can be multidirectional. See RAxis and RMixedAxis. 
 *
 * @{
 */

template<class Axis>
class RState : public UniversalState
{
public:
  typedef Axis axis;

  //! Construct a state with the name.
  RState (const char* name);
  RState(uint32_t);
  RState(const UniversalState& us)
    : RState((uint32_t) us) {}

  //! Create RState as a current state of `obj'. The Axis
  //! matching is checked in a compilation time.
  RState(const ObjectWithStatesInterface<Axis>& obj);

  std::string name () const;

  bool operator== (const RState<Axis>& st) const
  { return the_state == st.the_state; }

  bool operator!= (const RState<Axis>& st) const
  { return the_state == st.the_state; }

  template<class DerivedAxis>
  operator RState<DerivedAxis> () const;
};

template<class Axis, class Axis2> 
class RMixedAxis;

//! @class RAxis
template<class Axis>
using RAxis = RMixedAxis<Axis, Axis>;

//! It serves stateMap for RMixedAxis
//! (only one map per main Axis)
template<class Axis>
class StateMapInstance 
{
public:
  static StateMap* get_map()
  { 
    init();
    return stateMap; 
  }

  static StateMapId get_map_id()
  { 
    init();
    return id; 
  }

protected:
  // <NB> no explicit default values
  // (to prevent overwrite by static init code)
  static StateMap* stateMap;
  static std::atomic<StateMapId> id;

private:
  //! initialize the stateMap member if it is nullptr
  StateMapInstance() { }
  StateMapInstance(const StateMapInstance&) = delete;
  ~StateMapInstance() = delete;
  StateMapInstance& operator=(const StateMapInstance&) = 
    delete;

  static void init();
  static void init_impl();
};

//! Dummy StateAxis specialization of StateMapInstance
//! (to allow recursive init).
template<>
class StateMapInstance<StateAxis> 
{
public:
  static StateMapId get_map_id()
  { return 0; }
};

/**
 * A main class to working with states.
 * \tparam Axis - an axis where states are expressed.
 * \tparam Axis2 - an axis defined in
 * ObjectWithStatesInterface. Typically Axis2 is ancestor
 * or the same as Axis.
 */
template<class Axis, class Axis2>
class RMixedAxis : public Axis
{
public:
  typedef Axis axis;

  //! Check permission of moving obj to the `to' state.
  static void check_moving_to 
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& to);

  //! Atomic move obj to a new state
  static void move_to
    (ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& to);

  //! Atomic compare-and-change the state
  //! \return false if the object was not in `from' state,
  //! otherwise return true if the transition was done.
  //! \throw InvalidStateTransition if threre is no
  //! transition from->to and the object is in the `from'
  //! state.  
  static bool compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& from,
     const RState<Axis>& to);
	 
#if 0
  //! Compare-and-change the state for several object. It
  //! is not atomic for the bunch but it is atomic for each
  //! object. In a case of some object is not moved it
  //! rollbacks all changes previously done or call
  //! ObjectWithStatesInterface::state_is_broken on all
  //! failed objects.
  //! \return false if the object was not in `from' state,
  //! otherwise return true if the transition was done.
  //! \throw InvalidStateTransition if threre is no
  //! transition from->to for at least one object. It
  //! guarantees all changes rollbacked.
  template<class Collection>
  static bool compare_and_move
    (Collection& objs, 
     const RState<Axis>& from,
     const RState<Axis>& to);
#endif
	 
  //! Atomic compare-and-change state which uses a
  //! set of possible from-states.
  //! \return false if the object was not in `from' states,
  //! otherwise return true if the transition was done.
  //! \throw InvalidStateTransition if threre is no
  //! transition current->to and the current state is in
  //! the `from' set.
  static bool compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const std::set<RState<Axis>>& from_set,
     const RState<Axis>& to);

  //! Atomic compare-and-change state which use rules from
  //! the trs map and tries to find an appropriate map
  //! element and change a state pair::first ->
  //! pair::second
  //! (if the current state is pair::first). 
  //! \return The constant interator to performed
  //! transition from trs or trs.end() if no transition
  //! was performed.
  static auto compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const std::map<const RState<Axis>, 
     const RState<Axis>>& trs
     ) -> typename std::remove_reference
            <decltype(trs)>::type::const_iterator;

  //! Atomic compare-and-change the state
  //! \return false if the object was in `not_from' state,
  //! otherwise return true if the transition was done.
  //! \throw InvalidStateTransition if threre is no
  //! transition from->to and the object is in the `from'
  //! state.  
  static bool neg_compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& not_from,
     const RState<Axis>& to);
	 
  static uint32_t state
    (const ObjectWithStatesInterface<Axis2>& obj);

  //! Atomic check the object state
  static bool state_is
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& st);

  //! Atomic check the obj state is in the set
  static bool state_in
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const std::initializer_list<RState<Axis>>& set);

  //! Atomic check the obj state is in the set
  template<template <class...> class Cont>
  static bool state_in
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const Cont<RState<Axis>>& set);

  //! Raise InvalidState when a state is not expected.
  static void ensure_state
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& expected);

  //! Raise InvalidState when a state is not in set
  static void ensure_state_in
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const std::initializer_list<RState<Axis>>& set);

  static const StateMap* state_map();

protected:
  RMixedAxis(const StateMapPar<Axis>& par);

private:
  RMixedAxis();
  RMixedAxis(const RMixedAxis&) = delete;
  //~RMixedAxis() = delete;
  RMixedAxis& operator=(const RMixedAxis&) = delete;

  typedef Logger<RAxis<Axis>> log;
};

//! RMixedAxis<Axis,Axis2>::state_is adapter
template<class T, class Axis2 = typename T::axis>
bool state_is
  (T& obj, 
   const curr::RState<typename T::State::axis>& to)
{
  return curr::RMixedAxis<typename T::State::axis, Axis2>
    :: state_is(obj, to);
}

//! RMixedAxis<Axis,Axis2>::move_to adapter
template<class T, class Axis2 = typename T::axis>
void move_to
  (T& obj, 
   const curr::RState<typename T::State::axis>& to)
{
  curr::RMixedAxis<typename T::State::axis, Axis2>
    :: move_to(obj, to);
}

//! RMixedAxis<Axis,Axis2>::compare_and_move adapter
template<class T, class Axis2 = typename T::axis>
bool compare_and_move
  (T& obj, 
   const curr::RState<typename T::State::axis>& from,
   const curr::RState<typename T::State::axis>& to)
{
  return curr::RMixedAxis<typename T::State::axis, Axis2>
    :: compare_and_move(obj, from, to);
}

//! RMixedAxis<Axis,Axis2>::compare_and_move adapter
template<class T, class Axis2 = typename T::axis>
bool compare_and_move
  (T& obj, 
   const std::set
     <curr::RState<typename T::State::axis>>& from_set,
   const curr::RState<typename T::State::axis>& to)
{
  return curr::RMixedAxis<typename T::State::axis, Axis2>
    :: compare_and_move(obj, from_set, to);
}

//! RMixedAxis<Axis,Axis2>::neg_compare_and_move adapter
template<class T, class Axis2 = typename T::axis>
bool neg_compare_and_move
  (T& obj, 
   const curr::RState<typename T::State::axis>& from,
   const curr::RState<typename T::State::axis>& to)
{
  return curr::RMixedAxis<typename T::State::axis, Axis2>
    :: neg_compare_and_move(obj, from, to);
}

template<class Axis, class Axis2> class RMixedEvent;

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

//! Wait is_from_event then perform 
//! RMixedAxis<Axis,Axis2>::compare_and_move 
template<class T, class Axis2 = typename T::axis>
void wait_and_move
  (T& obj, 
   const REvent<typename T::State::axis>& is_from_event,
   const RState<typename T::State::axis>& to,
   int wait_m = -1);

//! Wait is_from_event then perform 
//! RMixedAxis<Axis,Axis2>::compare_and_move
template<class T>
void wait_and_move
  (T& obj, 
   const std::set
     <RState<typename T::State::axis>>& from_set,
   const CompoundEvent& is_from_event,
   const RState<typename T::State::axis>& to,
   int wait_m = -1);

template<class T>
void wait_and_move
  (T& obj, 
   const std::map <
     RState<typename T::State::axis>, 
     RState<typename T::State::axis>
   >& trs,
   const CompoundEvent& is_from_event,
   int wait_m = -1);

#define DEFINE_AXIS_NS_TEMPL0(axis, templ, pars...)	\
  templ \
  const std::atomic<uint32_t>& axis::current_state    \
    (const curr::AbstractObjectWithStates* obj) const \
  { \
    return dynamic_cast<const \
      curr::RObjectWithStates<axis>*>(obj) \
        -> curr::RObjectWithStates<axis>::current_state \
          (*this);  \
  } \
  \
  templ \
  std::atomic<uint32_t>& axis::current_state \
    (curr::AbstractObjectWithStates* obj) const \
  { \
    return dynamic_cast<curr::RObjectWithStates<axis>*> \
      (obj) -> curr::RObjectWithStates<axis> \
        ::current_state(*this);  \
  } \
  \
  templ \
  void axis::update_events \
    (curr::AbstractObjectWithEvents* obj,       \
     curr::TransitionId trans_id,               \
      uint32_t to) \
  { \
    return dynamic_cast<curr::RObjectWithEvents<axis>*> \
       (obj)   \
      -> curr::RObjectWithEvents<axis>::update_events \
      (*this, trans_id, to); \
  } \
  \
  templ \
  void axis::state_changed \
    (curr::AbstractObjectWithStates* subscriber,   \
     curr::AbstractObjectWithStates* publisher,    \
     const curr::StateAxis& state_ax, \
     const UniversalState& new_state)             \
  { \
    return dynamic_cast<curr::RObjectWithStates<axis>*>  \
    (subscriber) \
      -> curr::RObjectWithStates<axis>::state_changed \
      (*this, state_ax, publisher, new_state);        \
  } \
  templ \
  curr::StateMapPar<axis> axis::get_state_map_par()   \
  {	\
    return curr::StateMapPar<axis>(pars,                \
      curr::StateMapInstance<typename axis::Parent> \
        ::get_map_id());  \
  } \
  templ \
  curr::UniversalState axis::bound(uint32_t st) const \
  { \
    return curr::RState<axis>(st);              \
  } 

#define DEFINE_AXIS_NS(axis, pars...) \
  DEFINE_AXIS_NS_TEMPL0(axis, , pars) 

#define DEFINE_AXIS_TEMPL(axis, templ_base, pars...) \
  DEFINE_AXIS_NS_TEMPL0 \
    (CURR_TEMPLATE_AND_PARS(axis, T, \
      CURR_ENABLE_BASE_TYPE(templ_base, T)), \
     template<class T>, pars)

#define DEFINE_AXIS(axis, pars...)	\
  DEFINE_AXIS_NS(axis, pars) \
  template class curr::RMixedAxis<axis, axis>;	\
  template class curr::RState<axis>;            \
  template class curr::RMixedEvent<axis, axis>;		

#define DECLARE_STATES(axis, state_class)	\
  typedef curr::RAxis<axis> state_class;  \
  friend curr::RAxis<axis>;

//! @deprecated RAxis is a ingleton now, this macro is
//! not necessary
#define DEFINE_STATES(axis)				

#define DECLARE_STATE_CONST(state_class, state)	\
  static const curr::RState<state_class::axis> \
    state ## State;

#define DEFINE_STATE_CONST(class_, state_class, state)	 \
  const curr::RState<typename class_::state_class::axis> \
    class_::state ## State(#state);

#define DECLARE_STATE_FUN(state_class, state) \
  static const curr::RState<state_class::axis>& \
    state ## Fun() \
  { \
    static const curr::RState< \
      typename state_class::axis> \
        the_state(#state); \
    return the_state; \
  }

#define STATE_OBJ(class_, action, object, state) \
  curr::RMixedAxis<typename class_::State::axis, \
             typename class_::axis>::action \
	 ((object), class_::state ## State)

#define A_STATE_OBJ(class_, axis_, action, object, state) \
  curr::RAxis<axis_>::action \
	 ((object), class_::state ## State)

#define STATE(class_, action, state) \
  STATE_OBJ(class_, action, *this, state)

#define A_STATE(class_, axis_, action, state)			\
  A_STATE_OBJ(class_, axis_, action, *this, state)

#define S(state) (state ## State)

#define CURR_STATE_DEF(axis, state) \
  const curr::RState<axis> S(state)(# state);

//! @}

}

#endif
