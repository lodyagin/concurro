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
#include <map>
#include <vector>
#include <initializer_list>
#include <typeindex>
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
  const UniversalState state;
  InvalidState(UniversalState st) : state(st) {}
  [[noreturn]] void raise(UniversalState expected) const;
};

struct NoSuchStateInMap : InvalidState
{
  const std::type_index map_axis;
  NoSuchStateInMap(
    UniversalState the_state,
    const StateMap& the_map
  );
  [[noreturn]] void raise() const;
};

struct InvalidStateTransition : InvalidState
{
  const UniversalState from, to;

  InvalidStateTransition 
    (UniversalState from_, 
     UniversalState to_)
    : InvalidState
    (to_/*, SFORMAT("Invalid state transition from ["
                  << from_ << "] to [" << to_ << "].")*/),
    from(from_), to(to_)
  {}

  [[noreturn]] void raise() const;
};

struct NoStateWithTheName : StateMapException
{
  ::types::constexpr_string the_name;

  NoStateWithTheName(
    ::types::constexpr_string name, 
    const StateMap* map
  );
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

struct BadParameters : StateMapException {};

//! @}



//! @addtogroup states
//! @{

/* StateMap class */

typedef int16_t StateMapId;

class StateMapParBase
{
public:
  std::list<::types::constexpr_string> states;
  std::list<
    std::pair<::types::constexpr_string, ::types::constexpr_string>> transitions;
  StateMapId parent_map;
  std::type_index axis_type_index;

  StateMapParBase
    (std::initializer_list<::types::constexpr_string> states_,
     std::initializer_list<
     std::pair<::types::constexpr_string, ::types::constexpr_string>> transitions_,
     StateMapId parent_map_,
     std::type_index axis
     )
  : states(states_), 
    transitions(transitions_),
    parent_map(parent_map_), 
    axis_type_index(axis)
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
    std::initializer_list<::types::constexpr_string> states,
    std::initializer_list<
    std::pair<::types::constexpr_string, ::types::constexpr_string>> transitions,
    StateMapId parent_map_ = 0 // default is top level
    )
    : StateMapParBase
        (states, transitions, parent_map_, 
         std::type_index(typeid(Axis)))
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
  uint32_t create_state(::types::constexpr_string name) 
    const
  {
    Name2Idx::const_iterator cit = name2idx.find(name);
    if (cit != name2idx.end ()) {
      return (numeric_id << STATE_MAP_SHIFT) 
        | STATE_IDX(cit->second);
    }
    else 
      throw NoStateWithTheName(name, this);
  }

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

  ::types::constexpr_string get_state_name(uint32_t state)
    const;

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
    (::types::constexpr_string from,
     ::types::constexpr_string to) const;
  
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
    return ::types::demangled_name(axis());
  }

  std::type_index axis() const
  {
    return axis_type_index;
  }

  const std::string universal_object_id;
  const int16_t numeric_id;

protected:

  typedef std::map<::types::constexpr_string, StateIdx>  
    Name2Idx;
  typedef std::vector<::types::constexpr_string> Idx2Name;

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
  //! The axis type
  const std::type_index axis_type_index;

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
