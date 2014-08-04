/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

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

#include <assert.h>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <boost/multi_array.hpp>
#include "types/exception.h"
#include "HasStringView.h"
#include "SCheck.h"
#include "Repository.h"

namespace curr {

//! @addtogroup states
//! @{

// <NB> the same size (see UniversalEvent)
typedef uint16_t StateIdx;
typedef uint16_t TransitionId;

class StateMap;
class UniversalEvent;

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

//! @}


//! @addtogroup exceptions
//! @{

struct StateMapException : std::exception {};

struct InvalidState : StateMapException
{
  InvalidState(UniversalState st) : state(st) {}
  const UniversalState state;
};

[[noreturn]] inline void throw_invalid_state(
  UniversalState current,
  UniversalState expected
)
{
  using namespace ::types;

  const auto_string<28> currentName = current.name();
  const auto_string<28> expectedName = expected.name();

  throw ::types::exception(
    InvalidState(current),
    "Invalid state [", currentName, "] when expected [",
    expectedName, "]"
  );
}

struct InvalidStateTransition : InvalidState
{
  InvalidStateTransition 
    (UniversalState from_, 
     UniversalState to_)
    : InvalidState
    (to_/*, SFORMAT("Invalid state transition from ["
                  << from_ << "] to [" << to_ << "].")*/),
    from(from_), to(to_)
  {}

  UniversalState from, to;
};

struct NoStateWithTheName : StateMapException
{
#if 0
  NoStateWithTheName(const std::string& name, 
                     const StateMap* map);
#endif
};

/**
 * Exception: two states are incompatible on StateMap
 * level (when try to use in some operation).
 */
struct IncompatibleMap : StateMapException
{
#if 0
public:
  IncompatibleMap ()
    : SException ("Incompatible map")
  {}
#endif
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
  { 
    throw ::types::exception
      <TransformObjectIsNotImplemented>();
  }

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
         ::types::type<Axis>::name())
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
  struct BadParameters : StateMapException
  {
#if 0
  public:
    BadParameters ()
      : SException ("Bad initialization parameters")
    {}

    BadParameters (const std::string& str)
      : SException 
      (std::string ("Bad initialization parameters: ")
       + str)
    {}
#endif
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

  void outString (std::ostream& out) const override;

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
