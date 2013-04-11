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

class RaceConditionInStates
  : public SException
{
public:
  RaceConditionInStates
    (uint32_t old, 
	  uint32_t from, 
     uint32_t to)
    : SException 
    (SFORMAT("Somebody had time to change "
				 << to << "->" << old
				 << " while we incorrectly set " << to))
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
};

template<class Axis>
std::ostream&
operator<< (std::ostream&, const RState<Axis>&);

template<class Axis>
class RAxis : public Axis, public SSingleton<RAxis<Axis>>
{
public:
  typedef Axis axis;

  RAxis(const StateMapPar<Axis>& par);

  //! Check permission of moving obj to the `to' state.
  static void check_moving_to 
	 (const ObjectWithStatesInterface<Axis>& obj, 
	  const RState<Axis>& to);

  //! Atomic move obj to a new state
  static void move_to
	 (ObjectWithStatesInterface<Axis>& obj, 
	  const RState<Axis>& to);

  //! Atomic compare-and-change the state
  //! \return true if the object in the `from' state and
  //! false otherwise.
  //! \throw InvalidStateTransition if threre is no
  //! transition from->to and the object is in the `from'
  //! state.  
  static bool compare_and_move
	 (ObjectWithStatesInterface<Axis>& obj, 
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
	 (ObjectWithStatesInterface<Axis>& obj, 
	  const std::set<RState<Axis>>& from_set,
	  const RState<Axis>& to);
	 
  static uint32_t state
	 (const ObjectWithStatesInterface<Axis>& obj);

  //! Atomic check the object state
  static bool state_is
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const RState<Axis>& st);

  //! Atomic check the obj state is in the set
  static bool state_in
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const std::initializer_list<RState<Axis>>& set);

  //! Raise InvalidState when a state is not expected.
  static void ensure_state
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const RState<Axis>& expected);

  static const StateMap& state_map() 
  { 
	 return *stateMap; 
  }
protected:
  typedef Logger<RAxis<Axis>> log;

  static StateMap* stateMap;
  std::atomic_flag changed;
};

#define DECLARE_STATES(axis, state_class)	\
  typedef RAxis<axis> state_class; \
  friend class RAxis<axis>;


#define DEFINE_STATES(axis, pars...)	\
  static RAxis<axis> axis__(StateMapPar<axis>(pars));


#define DECLARE_STATE_CONST(state_class, state)	\
  static const RState<state_class::axis> state ## State;


#define DEFINE_STATE_CONST(class_, state_class, state)	\
  const RState<class_::state_class::axis>					\
    class_::state ## State(#state);

#endif
