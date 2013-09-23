/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
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

#include "StateMap.h"
#include "ObjectWithStatesInterface.h"
#include "SSingleton.h"
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

/*template<class Axis>
struct UnitializedAxis : public SException
{
UnitializedAxis()
  : SException(SFORMAT(
                 "The axis " << typeid(Axis).name()
                 << " must be initialized before usage"
                 ))
  {}
  
  };*/

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
: public SAutoSingleton<StateMapInstance<Axis>>
{
public:
  //! initialize the stateMap member if it is nullptr
  StateMapInstance() { id = init(); }

  StateMap* get_map() const
  { return stateMap; }

  StateMapId get_map_id() const
  { return id; }

protected:
  StateMap* stateMap;
  StateMapId id;

private:
  StateMapId init();
};

//! Dummy StateAxis specialization of StateMapInstance
//! (to allow recursive init).
template<>
class StateMapInstance<StateAxis> 
  : public SAutoSingleton<StateMapInstance<StateAxis>>
{
public:
  StateMapId get_map_id() const
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
class RMixedAxis : public Axis, 
  public SAutoSingleton<RAxis<Axis>>
{
public:
  typedef Axis axis;

  RMixedAxis();

  //! Check permission of moving obj to the `to' state.
  static void check_moving_to 
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& to);

  //! Atomic move obj to a new state
  static void move_to
    (ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& to);

  //! Atomic compare-and-change the state
  //! \return true if the object in the `from' state and
  //! false otherwise.
  //! \throw InvalidStateTransition if threre is no
  //! transition from->to and the object is in the `from'
  //! state.  
  static bool compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& from,
     const RState<Axis>& to);
	 
  //! Atomic compare-and-change the state which uses a
  //! set of possible from-states.
  //! \return true if the object in one of the `from'
  //! states and false otherwise.
  //! \throw InvalidStateTransition if threre is no
  //! transition current->to and the current state is in
  //! the `from' set.
  static bool compare_and_move
    (ObjectWithStatesInterface<Axis2>& obj, 
     const std::set<RState<Axis>>& from_set,
     const RState<Axis>& to);
	 
  //! Atomic compare-and-change the state
  //! \return true if the object NOT in the `from' state and
  //! false otherwise.
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
  static bool state_in
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const std::set<RState<Axis>>& set);

  //! Raise InvalidState when a state is not expected.
  static void ensure_state
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const RState<Axis>& expected);

  //! Raise InvalidState when a state is not in set
  static void ensure_state_in
    (const ObjectWithStatesInterface<Axis2>& obj, 
     const std::initializer_list<RState<Axis>>& set);

  static const StateMap* state_map() 
  { 
    static_assert(is_ancestor<Axis2, Axis>(), 
                  "This state mixing is invalid.");
    return StateMapInstance<Axis>::instance().get_map(); 
  }

protected:
  RMixedAxis(const StateMapPar<Axis>& par);

private:
  typedef Logger<RAxis<Axis>> log;
};

#define DEFINE_AXIS_NS(axis, pars...)	\
  const std::atomic<uint32_t>& axis::current_state    \
    (const curr::AbstractObjectWithStates* obj) const \
  { \
    return dynamic_cast<const \
      curr::RObjectWithStates<axis>*>(obj) \
        -> curr::RObjectWithStates<axis>::current_state \
          (*this);  \
  } \
  \
  std::atomic<uint32_t>& axis::current_state \
    (curr::AbstractObjectWithStates* obj) const \
  { \
    return dynamic_cast<curr::RObjectWithStates<axis>*> \
      (obj) -> curr::RObjectWithStates<axis> \
        ::current_state(*this);  \
  } \
  \
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
  curr::StateMapPar<axis> axis::get_state_map_par()   \
  {	\
    return curr::StateMapPar<axis>(pars,                \
      curr::StateMapInstance<typename axis::Parent> \
        ::instance().get_map_id());  \
  } \
  curr::UniversalState axis::bound(uint32_t st) const \
  { \
    return curr::RState<axis>(st);              \
  } 

#define DEFINE_AXIS(axis, pars...)	\
  DEFINE_AXIS_NS(axis, pars) \
  template class curr::RMixedAxis<axis, axis>;	\
  template class curr::RState<axis>;            \
  template class curr::RMixedEvent<axis, axis>;		

#define DECLARE_STATES(axis, state_class)	\
  typedef curr::RAxis<axis> state_class;  \
  friend curr::RAxis<axis>;

//! @deprecated RAxis is SAutoSingleton now, this macro is
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

//! @}

}

#endif
