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

// <NB> the same size (see UniversalEvent)
typedef uint16_t StateIdx;
typedef uint16_t TransitionId;

class StateMap;
//class EmptyStateMap;
//class UniversalState;

//! A state space axis abstract base. Real axises will be
//! inherited. <NB> StateAxises can't be "extended". Like
//! x, y, x in decart coordinates they are "finite". But
//! on RState inheritance new states on the same axes can
//! be added.
class StateAxis //: public UniversalState
{
protected:
  //! Return an empty state map
  //static StateMap* get_state_map();
};

#define STATE_MAP_MASK 0x7fff0000
#define STATE_MAP_SHIFT 16
#define STATE_IDX_MASK 0xffff
#define STATE_MAP(state) ((state) & STATE_MAP_MASK)
#define STATE_IDX(state) ((state) & STATE_IDX_MASK)

#if 0
class StateCoder
{
  friend class StateMap;

public:
/*  UniversalState () 
    : state_map (0), state_idx (0) 
	 {}*/

  //StateCoder(uint32_t coded);

  operator uint32_t() const
  {
	 return state;
  }
  
  // NB not virutal
  //bool operator== (const UniversalState& state2) const;
  //bool operator!= (const UniversalState& state2) const;

  //std::string name() const;
  
  uint32_t        state;
  //const StateMap* state_map;
  //StateIdx        state_idx;

protected:
  StateCoder (const StateMap* map, StateIdx idx);
};
#endif

#define MIXED_EVENT

class UniversalEvent
{
public:
  uint16_t   id;

  enum { Mask = 0x8000 };

#if 0
  enum class Type {Transition, Destination};

  Type type() const
  {
	 return (id & Mask) ? Type::Destination
		: Type::Transition;
  }
#endif

protected:
  UniversalEvent(TransitionId trans_id)
 	 : id(trans_id) {}
#ifdef MIXED_EVENT
UniversalEvent(uint32_t state, bool) 
    : id(state | Mask) {}
#endif
};

/*std::ostream& 
operator<< (std::ostream&, const UniversalState&);
*/

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
typedef std::string StateMapId;
#else
typedef int16_t StateMapId;
#endif

class StateMapParBase
{
public:
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

  virtual StateMapId get_id
  (const ObjectCreationInfo& oi) const = 0;

protected:
  StateMapId get_map_id(const ObjectCreationInfo&,
			const std::type_info&) const;
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

  StateMapId get_id(const ObjectCreationInfo& oi) const;
};

#if 0
struct EmptyStateMapPar : public StateMapParBase
{
  StateMap* create_derivation
    (const ObjectCreationInfo& oi) const;
};
#endif

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

  uint32_t create_state (const char* name) const;

  bool there_is_transition 
    (uint32_t from,
     uint32_t to) const;

  void check_transition
    (uint32_t from,
     uint32_t to) const;

  /* use ==
  bool is_equal
    (uint32_t a,
     uint32_t b) const;
  */
  
  //! Whether the state is compatible with the map.
  bool is_compatible(uint32_t state) const;

  std::string get_state_name
    (uint32_t state) const;

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
    (uint32_t from,
	  uint32_t to) const;

  TransitionId get_transition_id 
    (const char* from,
     const char* to) const;

  const std::string universal_object_id;
  const int16_t numeric_id;

protected:

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
  //StateMap();

  StateMap(const ObjectCreationInfo& oi,
			  const StateMapParBase& par);

private:
  typedef Logger<LOG::States> log;
};

#if 0
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
#endif

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
  friend class StateMapParBase;
public:
  typedef Repository< 
	 StateMap, 
	 StateMapParBase, 
	 std::unordered_map<StateMapId, StateMap*>, 
	 StateMapId > Parent;

  StateMapRepository() 
	 : Parent("StateMapRepository", 50), 
	 max_trans_id(0), last_map_id(0) {}

  //! Return max used transition id
  TransitionId max_transition_id() const
  { return max_trans_id; }

  //! Return a state map by a state axis
  StateMap* get_map_for_axis(const std::type_info& axis);

protected:
  //! Return a map id by an axis type.
  //! It creates new StateMapId 
  //! if it is new (unregistered) axis.
  StateMapId get_map_id(const std::type_info& axis);

  //! It is incremented in a StateMap constructor.
  std::atomic<TransitionId> max_trans_id;

  StateMapId last_map_id;
  std::map<std::string, StateMapId> axis2map_id;
};


template<class Axis>
StateMapId StateMapPar<Axis>
//
::get_id(const ObjectCreationInfo& oi) const
{
  return StateMapParBase::get_map_id(oi, typeid(Axis));
}


#endif
