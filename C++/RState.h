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


/**
 * RState is a state value (think about it as an extended
 * enum).
 *
 * \tparam Object an object which holds the state.
 * \tparam Axis a state axis.
 */
template<class Axis>
class RState : public Axis, protected UniversalState
{
public:
  //! Construct a state with the name.
  //! \param par states and transitions.
  RState (const StateMapPar<Axis>& par, 
			 const char* name);

  RState(uint32_t);

  RState(const ObjectWithStatesInterface<Axis>&);

  // ParentState& operator= (const UniversalState& us)
  void set_by_universal (uint32_t us);

#ifndef STATE_LOCKING //lock_state and then check on the
                      // universal state?

  /// Check permission of moving obj to the `to' state.
  static void check_moving_to 
	 (const ObjectWithStatesInterface<Axis>& obj, 
	  const RState& to);

#endif

  /// Atomiv move obj to a new state
  static void move_to
	 (ObjectWithStatesInterface<Axis>& obj, 
	  const RState& to);

#if 0
  /// Atomic move obj to a `to' state if it is not `to'
  static void move_if_not_already
	 (ObjectWithStatesInterface<Axis>& obj, 
	  const RState& to);
#endif

#ifdef STATE_LOCKING
  //! Return the current state and prohibit further state
  //! changes by acquiring a mutex till unlock_state
  //! call.
  static const UniversalState& lock_state
	 (const ObjectWithStatesInterface<Axis>& obj);
#else
  static uint32_t state
	 (const ObjectWithStatesInterface<Axis>& obj);
#endif

  //! Atomic check the object state
  static bool state_is
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const RState& st);

  //! Atomic check the obj state is in the set
  static bool state_in
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const std::initializer_list<RState>& set);

  //! Raise InvalidState when a state is not expected.
  static void ensure_state
	  (const ObjectWithStatesInterface<Axis>& obj, 
		const RState& expected);

  std::string name () const
  {
	  return this->stateMap->get_state_name (*this);
  }

  operator uint32_t() const
  {
	 return the_state;
  }

protected:
  typedef Logger<RState> log;

  static StateMap* stateMap;
  static StateMap* get_state_map() { return stateMap; }
};

template<class Axis>
std::ostream&
operator<< (std::ostream&, const RState<Axis>&);

#define DECLARE_STATES(/*class_,*/ axis, state_class)	\
  const static StateMapPar<axis>				\
    new_states__ ## state_class;				 \
  \
  typedef RState \
  </*class_,*/ axis/*, new_states__ ## state_class*/>	\
      state_class; \
  \
  friend class RState<axis>;
//  <class_, axis, new_states__ ## state_class>;


#define DEFINE_STATES(class_, axis, state_class)	\
  const StateMapPar<axis> \
	 class_::new_states__ ## state_class


#define DECLARE_STATE_CONST(state_class, state)	\
  static const state_class state ## State;


#define DEFINE_STATE_CONST(class_, state_class, state)	\
  const class_::state_class class_::state ## State \
    (class_::new_states__ ## state_class, #state);

#include "RState.hpp"

#endif
