#ifndef CONCURRO_STATEMAP_H_
#define CONCURRO_STATEMAP_H_

#include "SSingleton.h"
#include "HasStringView.h"
#include "Logging.h"
#include "SException.h"
#include "SCheck.h"
#include <assert.h>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <boost/multi_array.hpp>

typedef unsigned int StateIdx;

/*struct State2Idx
{
  StateIdx idx;
  const char* state;
};

struct StateTransition
{
  const char* from;
  const char* to;
};*/

class StateMap;
class UniversalState;

/// A state space axis abstract base. Real axises will be inherited.
/// <NB> StateAxises can't be "extended". Like x, y, x in decart
/// coordinates they are "finite". But on RState inheritance new
/// states on the same axes can be added.
class StateAxis //: public UniversalState
{
public:

 //virtual StateAxis& operator= (const UniversalState&) = 0;
//  virtual StateAxis& operator= (const StateAxis&) = 0;

  //virtual bool operator==(const StateAxis&) const = 0;

  virtual operator UniversalState () const = 0;
  virtual void set_by_universal (const UniversalState&) = 0;

protected:
  //StateAxis(StateMap* map_extension, const char* initial_state);

  /// Return an empty state map
  static StateMap* get_state_map();
};

class UniversalState
{
  friend class StateMap;

public:
  UniversalState () 
    : state_map (0), state_idx (0) 
  {}
  
  /*explicit UniversalState (const StateAxis& initial_state)
  {
	 *this = initial_state;
	 }*/

  //virtual ~UniversalState() {}

  // NB not virutal
  bool operator== (const UniversalState& state2) const;

  std::string name() const;
  
  const StateMap* state_map;
  StateIdx        state_idx;

protected:
  UniversalState (const StateMap* map, StateIdx idx)
    : state_map (map), state_idx (idx)
  {}
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

struct StateMapPar
{
  StateMapPar (
	 std::initializer_list<const char*> states_,
	 std::initializer_list<std::pair<const char*, const char*>> transitions_
	 )
  : states(states_), transitions(transitions_) {}

  std::initializer_list<const char*> states;
  std::initializer_list<std::pair<const char*, const char*>> transitions;
};

std::ostream& operator<< (std::ostream& out, const StateMapPar& par);

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

  StateMap(const StateMap* parent, const StateMapPar& new_states);

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

  StateIdx get_n_states() const
  {
	 return n_states;
  }

  virtual bool is_empty_map () const
  {
	 return false;
  }

  // overrides
  void outString (std::ostream& out) const;

protected:

  /// Construct an empty state map
  StateMap();

  struct IdxTransRec
  {
    IdxTransRec () : from (0), to (0) {}
    StateIdx from;
    StateIdx to;
  };

  typedef std::unordered_map<const char*, StateIdx>  Name2Idx;
#if 0
  typedef std::map<StateIdx, std::string> Idx2Name;
#else
  typedef std::vector<const char*> Idx2Name;
#endif

  //typedef std::vector< std::vector <int> > Trans2Number;
  //typedef std::vector<IdxTransRec> Number2Trans;

  const StateIdx n_states;
  Name2Idx     name2idx;
  Idx2Name     idx2name;  
  //Trans2Number trans2number;
  //Number2Trans number2trans;
  typedef boost::multi_array<bool, 2> Transitions;
  Transitions transitions;

  int get_transition_id 
    (const UniversalState& from,
     const UniversalState& to) const;

private:
  typedef Logger<LOG::States> log;

  // Is used from constructor
  // Fills trans2number and number2trans
  void add_transitions 
    (std::initializer_list<std::pair<const char*, const char*>> transitions);
};

class EmptyStateMap : public StateMap, public SAutoSingleton<EmptyStateMap>
{
public:
  EmptyStateMap() {}

  virtual bool is_empty_map () const
  {
	 return true;
  }
};

/**
 * RState is a state value (think about it as an extended enum).
 * \tparam Object an object which holds the state.
 * \tparam 
 */
template <class Object, class ParentState, StateMapPar const& par>
class RState 
: public ParentState, 
  virtual protected UniversalState
{
public:
  /// Construct a state with the name.
  RState (const char* name);

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

  operator UniversalState () const /* overrides */
  {
	 return *this;
  }

protected:
  static StateMap* stateMap;
  typedef Logger<RState> log;

  static StateMap* get_state_map() { return stateMap; }
};

template <class Object, class ParentState, StateMapPar const& par>
StateMap* RState<Object, ParentState, par>::stateMap = 0;

template <class Object, class ParentState, StateMapPar const& par>
RState<Object, ParentState, par>::RState (const char* name)
{
  assert (name);

  if (!stateMap)
    stateMap = new StateMap(ParentState::get_state_map(), par);

  *((UniversalState*) this) = stateMap->create_state (name);
}

template <class Object, class ParentState, StateMapPar const& par>
void RState<Object, ParentState, par>
//
::check_moving_to(const Object& obj, const RState& to)
{
	RState from = to;
	obj.state(from);
	from.stateMap->check_transition (from, to);
}

template <class Object, class ParentState, StateMapPar const& par>
void RState<Object, ParentState, par>
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

template <class Object, class ParentState, StateMapPar const& par>
bool RState<Object, ParentState, par>
//
::state_is (const Object& obj, const RState& st)
{
#if 0
	RState current = st;
	obj.state (current);
	return current == st;
#else
	return obj.state_is(st);
#endif
}


#endif
