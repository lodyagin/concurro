// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSTATE_H_
#define CONCURRO_RSTATE_H_

#include "StateMap.h"
#include "ObjectWithStatesInterface.h"

template<class Axis>
struct UnitializedAxis : public SException
{
  UnitializedAxis()
	 : SException(SFORMAT(
						 "The axis " << typeid(Axis).name()
						 << " must be initialized before usage"
						 ))
  {}
  
};

template<class Axis>
class RState : public UniversalState
{
public:
  typedef Axis axis;

  //! Construct a state with the name.
  RState (const char* name);
  RState(uint32_t);
  RState(const ObjectWithStatesInterface<Axis>& obj);
  std::string name () const;

  template<class DerivedAxis>
  operator RState<DerivedAxis> () const;
};

template<class Axis, class Axis2> 
class RMixedAxis;

template<class Axis>
using RAxis = RMixedAxis<Axis, Axis>;


//! Server stateMap for RMixedAxis
//! (only one map per main Axis)
template<class Axis>
struct StateMapInstance
{
  static StateMap* stateMap;
  //! initialize the stateMap member if it is nullptr
  static StateMapId init();
};

//! Dummy StateAxis specialization of StateMapInstance
//! (to alloc recursive init).
template<>
struct StateMapInstance<StateAxis> 
{
  static StateMapId init() { return 0; }
};

template<class Axis, class Axis2>
class RMixedAxis : public Axis, 
  public SSingleton<RAxis<Axis>>
{
public:
  typedef Axis axis;

  RMixedAxis();

  /*RMixedAxis(
	 std::initializer_list<const char*> states,
	 std::initializer_list<
	 std::pair<const char*, const char*>> transitions
	 );*/

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
	 
  //! Atomic compare-and-change the state which uses a set
  //! of possible from-states.
  //! \return true if the object in one of the `from'
  //! states and false otherwise.
  //! \throw InvalidStateTransition if threre is no
  //! transition current->to and the current state is in
  //! the `from' set.
  static bool compare_and_move
	 (ObjectWithStatesInterface<Axis2>& obj, 
	  const std::set<RState<Axis>>& from_set,
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

  //! Raise InvalidState when a state is not expected.
  static void ensure_state
	  (const ObjectWithStatesInterface<Axis2>& obj, 
		const RState<Axis>& expected);

  static const StateMap& state_map() 
  { 
	 static_assert(is_ancestor<Axis2, Axis>(), 
						"This state mixing is invalid.");
	 return *StateMapInstance<Axis>::stateMap; 
  }
protected:
  typedef Logger<RAxis<Axis>> log;

  //std::atomic_flag changed;

  RMixedAxis(const StateMapPar<Axis>& par);
};

#define DECLARE_AXIS(axis, parent, pars...)		\
struct axis : public parent \
{ \
  typedef parent Parent; \
  static axis self_; \
  static StateMapPar<axis> get_state_map_par() \
  {	\
	 return StateMapPar<axis>(pars, \
	   StateMapInstance<typename axis::Parent>::init()); \
  } \
};	\
template class RMixedAxis<axis, axis>;	\
template class RState<axis>; \
template class RMixedEvent<axis, axis>;		
	

#define DECLARE_STATES(axis, state_class)	\
  typedef RAxis<axis> state_class; \
  friend RAxis<axis>;

#define DEFINE_STATES(axis)				\
  static RAxis<axis> raxis__ ## axis; 

#define DECLARE_STATE_CONST(state_class, state)	\
  static const RState<state_class::axis> state ## State;


#define DEFINE_STATE_CONST(class_, state_class, state)	\
  const RState<class_::state_class::axis>					\
    class_::state ## State(#state);

#define STATE_OBJ(class_, action, object, state) \
  RMixedAxis<typename class_::State::axis, \
             typename class_::axis>::action \
	 ((object), class_::state ## State)

#define A_STATE_OBJ(class_, axis_, action, object, state) \
  RAxis<axis_>::action \
	 ((object), class_::state ## State)

#define STATE(class_, action, state) \
  STATE_OBJ(class_, action, *this, state)

#define A_STATE(class_, axis_, action, state)			\
  A_STATE_OBJ(class_, axis_, action, *this, state)

#endif
