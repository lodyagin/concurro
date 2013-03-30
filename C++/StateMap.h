// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_STATEMAP_H_
#define CONCURRO_STATEMAP_H_

#include "SSingleton.h"
#include "HasStringView.h"
#include "Logging.h"
#include "SException.h"
#include "SCheck.h"
#include "Repository.h"
#include <assert.h>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <boost/multi_array.hpp>

typedef unsigned int StateIdx;

typedef uint16_t TransitionId;

class StateMap;
class EmptyStateMap;
class UniversalState;

//! A state space axis abstract base. Real axises will be
//! inherited. <NB> StateAxises can't be "extended". Like
//! x, y, x in decart coordinates they are "finite". But
//! on RState inheritance new states on the same axes can
//! be added.
class StateAxis //: public UniversalState
{
public:

 //virtual StateAxis& operator= (const UniversalState&) = 0;
//  virtual StateAxis& operator= (const StateAxis&) = 0;

  //virtual bool operator==(const StateAxis&) const = 0;
  
#if 0
  static const std::type_info& axis()
  {
	 return typeid(StateAxis);
  }

  virtual operator UniversalState () const = 0;
  virtual void set_by_universal (const UniversalState&) = 0;
#endif

protected:
  //StateAxis(StateMap* map_extension, const char* initial_state);

  //! Return an empty state map
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

#if 0
//! StateMap is identified by Object and Axis.
typedef std::pair<
  const std::type_info&, 
  const std::type_info&
  > StateMapId;

class StateMapId
{
public:
  StateMapId(const std::type_info& ti)
	 : name(ti.name()) {}
protected:
  std::string name;
};
#else
typedef std::string StateMapId;
#endif

//bool operator< (const std::type_info&, const std::type_info&);

//std::ostream&
//operator<< (std::ostream&, const StateMapId&);

struct StateMapParBase// : public GeneralizedPar<StateMap>
{
  std::initializer_list<const char*> states;
  std::initializer_list<
    std::pair<const char*, const char*>> transitions;

  StateMapParBase (
	 std::initializer_list<const char*> states_,
	 std::initializer_list<
	   std::pair<const char*, const char*>> transitions_
	 )
	 : states(states_), transitions(transitions_) {}

  virtual StateMap* create_derivation
    (const ObjectCreationInfo& oi) const;

  virtual StateMap* transform_object
    (const StateMap*) const
  { THROW_NOT_IMPLEMENTED; }

  virtual StateMapId get_id() const = 0;
};

template<class Axis>
class StateMapPar : public StateMapParBase
{
public:
  // TODO add complex states (with severas axises).
  StateMapPar (
	 std::initializer_list<const char*> states,
	 std::initializer_list<
	   std::pair<const char*, const char*>> transitions
	 )
	 : StateMapParBase(states, transitions) {}

  StateMapId get_id() const
  {
	 return typeid(Axis).name();
  }
};

struct EmptyStateMapPar : public StateMapParBase
{
  StateMap* create_derivation
    (const ObjectCreationInfo& oi) const;
};

std::ostream& operator<< 
(std::ostream& out, const StateMapParBase& par);

class StateMapRepository;

class StateMap : public HasStringView,
  public SNotCopyable
{
  friend class StateMapParBase;
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

  const std::string& universal_id() const
  {
	 return universal_object_id;
  }

  TransitionId get_transition_id 
    (const UniversalState& from,
	  const UniversalState& to) const;

  TransitionId get_transition_id 
    (const char* from,
     const char* to) const;

protected:

  const std::string universal_object_id;

  typedef std::unordered_map<const char*, StateIdx>  
	 Name2Idx;
  typedef std::vector<const char*> Idx2Name;

  const StateIdx n_states;
  Name2Idx     name2idx;
  Idx2Name     idx2name;  
  typedef boost::multi_array<TransitionId, 2> Transitions;
  Transitions transitions;
  StateMapRepository* repo;

  //! Construct an empty state map
  StateMap();

  /*StateMap(const StateMap* parent, 
	 const StateMapPar& new_states);*/

  StateMap(const ObjectCreationInfo& oi,
			  const StateMapParBase& par);

  /*struct IdxTransRec
  {
    IdxTransRec () : from (0), to (0) {}
    StateIdx from;
    StateIdx to;
	 };*/

private:
  typedef Logger<LOG::States> log;
};

class EmptyStateMap 
 : public StateMap, public SAutoSingleton<EmptyStateMap>
{
public:
  EmptyStateMap() {}

  virtual bool is_empty_map () const
  {
	 return true;
  }
};

/**
 * A repository for state maps. It also maintains
 * additional global data.
 */
class StateMapRepository 
: public Repository<
  StateMap, 
  StateMapParBase, 
  std::unordered_map<StateMapId, StateMap*>,
  StateMapId
  >,
  public SAutoSingleton<StateMapRepository>
{
  friend class StateMap;
public:
  typedef Repository< 
	 StateMap, 
	 StateMapParBase, 
	 std::unordered_map<StateMapId, StateMap*>, 
	 StateMapId > Parent;

  StateMapRepository() 
	 : Parent("StateMapRepository", 50), 
	 max_trans_id(0) {}

  //! Return max used transition id
  TransitionId max_transition_id() const
  { return max_trans_id; }

protected:
  //! It is incremented in a StateMap constructor.
  std::atomic<TransitionId> max_trans_id;
};

#endif
