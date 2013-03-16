#ifndef CONCURRO_STATEMAP_H_
#define CONCURRO_STATEMAP_H_

#include "SSingleton.h"
#include "HasStringView.h"
#include "Logging.h"
#include "SException.h"
#include <assert.h>
#include <map>
#include <vector>

#define UNIMPL_STATES_INHERITANCE

typedef unsigned int StateIdx;

struct State2Idx
{
  StateIdx idx;
  const char* state;
};

struct StateTransition
{
  const char* from;
  const char* to;
};

class StateMap;

class UniversalState
{
  friend class StateMap;

public:
  UniversalState () 
    : state_map (0), state_idx (0) 
  {}

  /*bool operator==
    (const UniversalState& state2) const
  {
    return state_map == state2.state_map
      && state_idx == state2.state_idx;
		}*/

protected:
  UniversalState (const StateMap* map, StateIdx idx)
    : state_map (map), state_idx (idx)
  {}

  const StateMap* state_map;
  StateIdx        state_idx;
};

/* Exceptions */

class InvalidStateTransition 
  : public SException
{
public:
  InvalidStateTransition 
    (const std::string& from, 
     const std::string& to)
    : SException 
    (std::string ("Invalid state transition from [")
    + from
    + "] to ["
    + to
    + "]."
    )
  {}
};

class NoStateWithTheName : public SException
{
public:
  NoStateWithTheName ()
    : SException ("No state with the name")
  {}
};

class IncompatibleMap : public SException
{
public:
  IncompatibleMap ()
    : SException ("Incompatible map")
  {}
};

/* StateMap class */

class StateMap : 
   public HasStringView 
   //FIXME pointer comparison
{
public:
  class BadParameters : public SException
  {
  public:
    BadParameters ()
      : SException ("Bad initialization parameters")
    {}

    BadParameters (const std::string& str)
      : SException 
      (std::string ("Bad initialization parameters: ")
       + str)
    {}

  };

  StateMap 
    (const State2Idx new_states[], 
     const StateTransition transitions[]);

  // Return the number of states in the map.
  StateIdx size () const;

  UniversalState create_state (const char* name) const;

  bool there_is_transition 
    (const UniversalState& from,
     const UniversalState& to) const;

  void check_transition
    (const UniversalState& from,
     const UniversalState& to) const;

  bool is_equal
    (const UniversalState& a,
     const UniversalState& b) const;
  
  bool is_compatible 
    (const UniversalState& state) const;

  std::string get_state_name
    (const UniversalState& state) const;

  // overrides
  void outString (std::ostream& out) const;

protected:

  struct IdxTransRec
  {
    IdxTransRec () : from (0), to (0) {}
    StateIdx from;
    StateIdx to;
  };

  typedef std::map<std::string, StateIdx> 
    Name2Idx;
  typedef std::map<StateIdx, std::string> Idx2Name;

  typedef std::vector< std::vector <int> > Trans2Number;
  typedef std::vector<IdxTransRec> Number2Trans;

  Name2Idx     name2idx;
  Idx2Name     idx2name;  
  Trans2Number trans2number;
  Number2Trans number2trans;

  int get_transition_id 
    (const UniversalState& from,
     const UniversalState& to) const;

private:
  typedef Logger<LOG::States> log;

  // Is used from constructor
  // Fills trans2number and number2trans
  void add_transitions 
    (const StateTransition transitions[], 
     int nTransitions);
};

/// A state space axis abstract base. Real axises will be inherited.
/// <NB> StateAxises can't be "extended". Like x, y, x in decart
/// coordinates they are "finite". But on RState inheritance new
/// states on the same axes can be added.
class StateAxis {};

/**
 * RState is a state value (think about it as an extended enum).
 * \tparam Object an object which holds the state.
 * \tparam 
 */
template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
class RState : public ParentState, virtual protected UniversalState
{
public:
  /// Construct a state with the name.
  RState (const char* name);

  bool operator==(const RState& st) const;

  /// Check permission of moving obj to the `to' state.
  static void check_moving_to 
	 (const Object& obj, const RState& to);

  /// Move obj to a new state
  static void move_to
	 (Object& obj, const RState& to);

  static bool state_is
	  (const Object& obj, const RState& st);

  std::string name () const
  {
	  return this->stateMap->get_state_name (*this);
  }

protected:
  static StateMap* stateMap;
};

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
StateMap* RState<Object, ParentState, new_states, transitions>::stateMap = 0;

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
RState<Object, ParentState, new_states, transitions>::RState (const char* name)
{
  assert (name);

  if (!stateMap)
#ifndef UNIMPL_STATES_INHERITANCE
    stateMap = new StateMap(allStates, allTrans);
#else
    stateMap = new StateMap(new_states, transitions);
#endif

  *((UniversalState*) this) = stateMap->create_state (name);
}

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
bool RState<Object, ParentState, new_states, transitions>
//
::operator== (const RState& st) const
{
	if (this->stateMap != st.stateMap)
		THROW_EXCEPTION(SException, 
		 "concurro: Compare states from different maps is not implemented");

	return this->state_idx == st.state_idx;
}

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
void RState<Object, ParentState, new_states, transitions>
//
::check_moving_to(const Object& obj, const RState& to)
{
	RState from = to;
	obj.state(from);
	from.stateMap->check_transition (from, to);
}

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
void RState<Object, ParentState, new_states, transitions>
//
::move_to(Object& obj, const RState& to)
{
	std::string fromName;
	if (LOG4CXX_UNLIKELY(Object::log::logger()->isDebugEnabled())) {
		RState from = to;
		obj.state(from);
		fromName = from.name();
	}

	check_moving_to (obj, to);
	obj.set_state_internal (to);

	LOG_DEBUG(Object::log, "Change state from [" << fromName << "] to [" 
				 << to.name() << "]");
}

template <
  class Object,
  class ParentState,
  const State2Idx new_states[], 
  const StateTransition transitions[]
>
bool RState<Object, ParentState, new_states, transitions>
//
::state_is (const Object& obj, const RState& st)
{
	RState current = st;
	obj.state (current);
	return current == st;
}


#endif
