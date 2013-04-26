// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_STATEMAP_H_
#define CONCURRO_STATEMAP_H_

#include "SSingleton.h"
#include "HasStringView.h"
#include "Logging.h"
#include "SException.h"
#include "SCheck.h"
#include "Repository.hpp"
#include <assert.h>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <boost/multi_array.hpp>

// <NB> the same size (see UniversalEvent)
typedef uint16_t StateIdx;
typedef uint16_t TransitionId;

class UniversalEvent;

class UniversalState
{
  friend class UniversalEvent;
  friend std::ostream& 
  operator<< (std::ostream& out, const UniversalState& st);

public:
  UniversalState() : the_state(0) {}
  UniversalState(uint32_t st) : the_state(st) {}
  operator uint32_t() const { return the_state; }

protected:
  uint32_t the_state;

  std::string name() const; 
};

std::ostream&
operator<< (std::ostream& out, const UniversalState& st);


class StateMap;
//class UniversalState;

//! A state space axis abstract base. Real axises will be
//! inherited. <NB> StateAxises can't be "extended". Like
//! x, y, x in decart coordinates they are "finite". But
//! on RState inheritance new states on the same axes can
//! be added.
class StateAxis {};

#define STATE_MAP_MASK 0x7fff0000
#define STATE_MAP_SHIFT 16
#define STATE_IDX_MASK 0x7fff
//! Return a state map id by a state.
#define STATE_MAP(state) \
  (((state) & STATE_MAP_MASK) >> STATE_MAP_SHIFT)
#define STATE_IDX(state) ((state) & STATE_IDX_MASK)

class UniversalEvent
{
public:
  //! arrival events have this bit set
  enum { Mask = 0x8000 };
  class NeedArrivalType: public SException
  {
  public:
    NeedArrivalType()
	 : SException
		("Need an arrival event type here") {}
  };

  //! Whether this event is of an 'arrival' and not
  //! 'transitional' type.
  bool is_arrival_event() const { return (id & Mask); }

  //! Transform arrival events to a state. Can throw
  //! NeedArrivalType. 
  operator UniversalState() const;

  bool operator==(UniversalEvent b) const
  {
	 return id == b.id;
  }

  bool operator!=(UniversalEvent b) const
  {
	 return id != b.id;
  }

  //! Construct a `transitional' type of an event
  UniversalEvent(TransitionId trans_id) : id(trans_id) {}
  //! Construct an `arrival' type of an event
  UniversalEvent(uint32_t state, bool) : id(state|Mask) 
  {
	 assert(STATE_MAP(id)); //must contain a state_map part
  }

  //uint16_t local_id() const { return STATE_IDX(id); }

  //! Bot transition and arrival ids are global.
  uint32_t global_id() const { return id; }

  //! A name as "<state>" or "<state>-><state>"
  std::string name() const; 

protected:
  uint32_t   id;

  uint32_t as_arrival() const
  { 
	 assert (is_arrival_event());
	 return id & ~Mask; 
  }
};

std::ostream&
operator<< (std::ostream& out, const UniversalEvent& ue);


/* Exceptions */

class InvalidState : public SException
{
public:
  InvalidState(UniversalState current,
					UniversalState expected);
  InvalidState(const std::string& msg)
	 : SException(msg) {}
};

class InvalidStateTransition 
  : public InvalidState
{
public:
  InvalidStateTransition 
    (UniversalState from_, 
     UniversalState to_)
    : InvalidState
    (SFORMAT("Invalid state transition from ["
				 << from_ << "] to [" << to_ << "].")) ,
	 from(from_), to(to_)
  {}

  UniversalState from, to;
};

class NoStateWithTheName : public SException
{
public:
  NoStateWithTheName(const std::string& name, 
							const StateMap* map);
};

class IncompatibleMap : public SException
{
public:
  IncompatibleMap ()
    : SException ("Incompatible map")
  {}
};

/* StateMap class */

typedef int16_t StateMapId;

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
  (ObjectCreationInfo& oi) const = 0;

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

  StateMapId get_id(ObjectCreationInfo& oi) const;
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

  uint32_t create_state (const char* name) const;

  bool there_is_transition 
    (uint32_t from,
     uint32_t to) const;

  void check_transition
    (uint32_t from,
     uint32_t to) const;

  //! Whether the state is compatible with the map.
  bool is_compatible(uint32_t state) const;

  std::string get_state_name(uint32_t state) const;

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
  
  //! Return states by a transition id
  void get_states(TransitionId trans_id,
						uint32_t& from, uint32_t& to);

  const std::string universal_object_id;
  const int16_t numeric_id;

protected:

  typedef std::unordered_map<std::string, StateIdx>  
	 Name2Idx;
  typedef std::vector<std::string> Idx2Name;

  typedef std::map<TransitionId, 
	 std::pair<StateIdx, StateIdx>> Transition2States;

  const StateIdx n_states;
  Name2Idx     name2idx;
  Idx2Name     idx2name;  
  typedef boost::multi_array<TransitionId, 2> Transitions;
  Transitions transitions;
  Transition2States trans2states;
  StateMapRepository* repo;

  StateMap(const ObjectCreationInfo& oi,
			  const StateMapParBase& par);

private:
  typedef Logger<LOG::States> log;
};

/**
 * A repository for state maps. It also maintains
 * additional global data.
 */
class StateMapRepository 
: public Repository<
  StateMap, 
  StateMapParBase, 
  std::unordered_map,
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
	 std::unordered_map,
	 StateMapId > Parent;

  StateMapRepository() 
	 : Parent("StateMapRepository", 50), 
	 max_trans_id(0), last_map_id(0) {}

  //! Return max used transition id
  TransitionId max_transition_id() const
  { return max_trans_id; }

  //! Return a state map by a state axis
  StateMap* get_map_for_axis(const std::type_info& axis);

  std::string get_state_name(uint32_t state) const;

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
::get_id(ObjectCreationInfo& oi) const
{
  return StateMapParBase::get_map_id(oi, typeid(Axis));
}


#endif
