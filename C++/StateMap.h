/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_STATEMAP_H_
#define CONCURRO_STATEMAP_H_

//#include "SSingleton.h"
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

namespace curr {

//! @addtogroup states
//! @{

// <NB> the same size (see UniversalEvent)
typedef uint16_t StateIdx;
typedef uint16_t TransitionId;

class StateMap;
class UniversalEvent;

class UniversalState
{
  friend class UniversalEvent;
  friend std::ostream& 
  operator<< (std::ostream& out, const UniversalState& st);

public:
  UniversalState() : the_state(0), the_map(nullptr) {}
  UniversalState(uint32_t st) 
    : the_state(st), the_map(nullptr) {}
  //! Change a map
  UniversalState(const StateMap* new_map, uint32_t st);
  operator uint32_t() const { return the_state; }

  //! It is a time-consuming comparison, use only if you
  //! understand what you do. In other cases use
  //! RState<Axis>::operator==.
  bool operator== (const UniversalState&) const;
  bool operator!= (const UniversalState&) const;

  //! Never use it if you already know a map (it makes a
  //! map-lookup). StateMap::get_state_name suit for most
  //! cases. 
  std::string name() const; 

protected:
  void init_map() const;
  uint32_t the_state;
  mutable const StateMap* the_map;
};

std::ostream&
operator<< (std::ostream& out, const UniversalState& st);

class AbstractObjectWithStates;
class AbstractObjectWithEvents;

class StateMap;
//class UniversalState;

//! Return true if DerivedAxis is same or derived from
//! Axis
template<class Axis, class DerivedAxis>
  constexpr bool is_ancestor()
{
  return std::is_base_of<Axis, DerivedAxis>::value;
}

#define STATE_MAP_MASK 0x7fff0000
#define STATE_MAP_SHIFT 16
#define STATE_IDX_MASK 0x7fff
#define EVENT_IDX_MASK 0xffff
//! Return a state map id by a state.
#define STATE_MAP(state)                                \
  (((state) & STATE_MAP_MASK) >> STATE_MAP_SHIFT)
#define STATE_IDX(state) ((state) & STATE_IDX_MASK)
#define EVENT_IDX(state) ((state) & EVENT_IDX_MASK)

class UniversalEvent
{
public:
  //! arrival events have this bit set
  enum { Mask = 0x8000 };

  //! Exception: Need an arrival event type here
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

  //! Both transition and arrival ids without a map.
  uint16_t local_id() const { return EVENT_IDX(id); }

  //! Both transition and arrival ids with a map.
  uint32_t global_id() const { return id; }

  //! A name as "<state>" or "<state>-><state>"
  std::string name() const; 

  uint32_t as_state_of_arrival() const
  { 
    assert (is_arrival_event());
    return id & ~Mask; 
  }

  TransitionId as_transition_id() const
  {
    assert(!is_arrival_event());
    return id;
  }

protected:
  uint32_t   id;
};

std::ostream&
operator<< (std::ostream& out, const UniversalEvent& ue);

//! @}


//! @addtogroup exceptions
//! @{

class InvalidState : public SException
{
public:
  InvalidState(UniversalState current,
               UniversalState expected);
  InvalidState(UniversalState st,
               const std::string& msg);
  const UniversalState state;
};

class InvalidStateTransition 
: public InvalidState
{
public:
  InvalidStateTransition 
    (UniversalState from_, 
     UniversalState to_)
    : InvalidState
    (to_, SFORMAT("Invalid state transition from ["
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

/**
 * Exception: two states are incompatible on StateMap
 * level (when try to use in some operation).
 */
class IncompatibleMap : public SException
{
public:
  IncompatibleMap ()
    : SException ("Incompatible map")
  {}
};

//! @}



//! @addtogroup states
//! @{

/* StateMap class */

typedef int16_t StateMapId;

class StateMapParBase
{
public:
  std::list<std::string> states;
  std::list<
    std::pair<std::string, std::string>> transitions;
  StateMapId parent_map;
  std::string axis_name;

  StateMapParBase
    (std::initializer_list<std::string> states_,
     std::initializer_list<
     std::pair<std::string, std::string>> transitions_,
     StateMapId parent_map_,
     const std::string& an_axis_name
     )
  : states(states_), transitions(transitions_),
    parent_map(parent_map_), axis_name(an_axis_name)
  {}

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
  // TODO add complex states (with several axises).
  StateMapPar (
    std::initializer_list<std::string> states,
    std::initializer_list<
    std::pair<std::string, std::string>> transitions,
    StateMapId parent_map_ = 0 // default is top level
    )
    : StateMapParBase
        (states, transitions, parent_map_, 
         curr::type<Axis>::name())
  {}

  StateMapId get_id(ObjectCreationInfo& oi) const;
};

std::ostream& operator<< 
(std::ostream& out, const StateMapParBase& par);

class StateMapRepository;

class StateMap : public HasStringView,
  public SNotCopyable
{
  friend class StateMapParBase;
  friend class StateMapRepository;
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
  //StateIdx size () const;

  uint32_t create_state (const char* name) const;

  bool there_is_transition 
    (uint32_t from,
     uint32_t to) const;

  void check_transition
    (uint32_t from,
     uint32_t to) const;

  //! Is this map the same or a descendant of `map2'
  bool is_same_or_descendant(const StateMap* map2) const;

  //! Whether the state is compatible with the map.
  bool is_compatible(uint32_t state) const;

  //! Ensure the state is compatible with the map.
  //! \throw IncompatibleMap
  void ensure_is_compatible(uint32_t state) const;

  std::string get_state_name(uint32_t state) const;

  StateIdx get_n_states() const
  {
    return n_states;
  }

  virtual bool is_empty_map () const
  {
    return parent == nullptr;
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

  TransitionId get_max_transition_id() const
  {
    return max_transition_id;
  }

  bool is_local_transition_arrival(StateIdx st) const
  {
    return local_transitions_arrivals.find(STATE_IDX(st))
      != local_transitions_arrivals.end();
  }

  std::string pretty_id() const
  {
    return axis_name;
  }

  const std::string universal_object_id;
  const int16_t numeric_id;

protected:

  typedef std::unordered_map<std::string, StateIdx>  
    Name2Idx;
  typedef std::vector<std::string> Idx2Name;

  typedef std::map<TransitionId, 
    std::pair<StateIdx, StateIdx>> Transition2States;

  StateMap* parent;
  const StateIdx n_states;
  Name2Idx     name2idx;
  Idx2Name     idx2name;  
  typedef boost::multi_array<TransitionId, 2> Transitions;
  Transitions transitions;
  Transition2States trans2states;
  StateMapRepository* repo;
  //! Max transition id in this map
  TransitionId max_transition_id;
  //! States participating in transitions unique for this
  //! map as destinations. 
  std::set<StateIdx> local_transitions_arrivals;
  //! The axis name
  std::string axis_name;

  StateMap(const ObjectCreationInfo& oi,
           const StateMapParBase& par);

  //! Empty map
  StateMap();

private:
  typedef Logger<LOG::States> log;
};


template<class Axis>
StateMapId StateMapPar<Axis>
//
::get_id(ObjectCreationInfo& oi) const
{
  return StateMapParBase::get_map_id(oi, typeid(Axis));
}

//! @}

}
#endif
