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

/**
 * RState is a state value (think about it as an extended
 * enum).
 *
 * \tparam Object an object which holds the state.
 * \tparam Axis a state axis.
 */
template<
//  class Object, 
  class Axis
#ifdef PARENT_MAP
  ,StateMapPar<Axis> const& par
#endif
>
class RState 
: public Axis, 
  virtual public UniversalState
{
public:
  //! Construct a state with the name.
  //! \param par states and transitions.
  RState (const StateMapPar<Axis>& par, const char* name);

  RState(const UniversalState&);

  // ParentState& operator= (const UniversalState& us)
  void set_by_universal (const UniversalState& us)
  {
	 // Different maps are not implemented
	 SCHECK(state_map == us.state_map);
	 state_idx = us.state_idx;
  }

  /*ParentState& operator= (const ParentState& st)
  {
	 *this = UniversalState(st);
	 }*/

  //bool operator==(const StateAxis& st) const /* overrides */;

  /// Check permission of moving obj to the `to' state.
  static void check_moving_to 
	 (const ObjectWithStatesInterface<Axis>& obj, const RState& to);

  /// Move obj to a new state
  static void move_to
	 (ObjectWithStatesInterface<Axis>& obj, const RState& to);

  static bool state_is
	  (const ObjectWithStatesInterface<Axis>& obj, const RState& st);

  std::string name () const
  {
	  return this->stateMap->get_state_name (*this);
  }

  operator UniversalState () const /* overrides */
  {
	 return *this;
  }

protected:
  static StateMap* stateMap;
  typedef Logger<RState> log;

  static StateMap* get_state_map() { return stateMap; }
};

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
