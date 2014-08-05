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

#ifndef CONCURRO_RSTATE_HPP_
#define CONCURRO_RSTATE_HPP_

#include <atomic>
#include "StateMapRepository.h"
#include "RState.h"
#include "RThread.hpp"
#include "Repository.hpp"
#include "RObjectWithStates.hpp"
#include "Event.h"

namespace curr {

template<class Axis>
void StateMapInstance<Axis>::init()
{
  if (!id) {
    static std::once_flag of;
    std::call_once(of, [](){ init_impl(); });
    assert(stateMap);
  }
  assert(id);
}

template<class Axis>
void StateMapInstance<Axis>::init_impl()
{
  StateMapInstance<typename Axis::Parent>::get_map_id();
  // ensure parent map ids are always less than childs
  // (it is used in StateMap::is_compatible)

  try {
    stateMap = StateMapRepository::instance()
      . get_map_for_axis(typeid(Axis));
  }
  catch(const StateMapRepository::NoSuchId&)
  {
    stateMap = StateMapRepository::instance()
      . create_object(Axis::get_state_map_par());
  }
  assert(stateMap);
  id = stateMap->numeric_id;
}

template<class Axis, class Axis2>
RMixedAxis<Axis, Axis2>
//
::RMixedAxis(/*const StateMapPar<Axis>& par*/)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  StateMapInstance<Axis>::get_map_id();
}

template<class Axis, class Axis2>
void RMixedAxis<Axis, Axis2>
//
::check_moving_to(
  const ObjectWithStatesInterface<Axis2>& obj, 
  const RState<Axis>& to)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  const uint32_t from = 
    const_cast<ObjectWithStatesInterface<Axis2>&> (obj)
    . current_state(Axis2::self()).load();
  StateMapInstance<Axis>::get_map()
    -> check_transition (STATE_IDX(from), STATE_IDX(to));
}

template<class Axis, class Axis2>
void RMixedAxis<Axis, Axis2>
//
::move_to(ObjectWithStatesInterface<Axis2>& obj, 
          const RState<Axis>& to)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  uint32_t from;
  TransitionId trans_id;
  auto& current = obj.current_state(Axis2::self());
  // <NB> get the logger before possible destruction of
  // obj (i.e. as a result of move to a terminal state)

  do {
    from = current.load();
    if (!(trans_id=
          StateMapInstance<Axis>::get_map()
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);
  } while (!current.compare_exchange_strong(from, to));
  
  if (!std::is_same<Axis, ConstructibleAxis>::value) 
    // prevent a deadlock
  {
    LOGGER_DEBUG(obj.logger(), 
                 "thread " 
                 << RThread<std::thread>::current_pretty_id()
                 << ">\t object " << obj.object_name()
                 << ">\t axis " << ::types::type<Axis>::name()
                 << "/" << ::types::type<Axis2>::name()
                 << ">\tmove_to:\t "
                 << UniversalState(from).name()
                 << std::hex << " (0x" << from
                 << ") -> " << to.name() << " (0x" 
                 << (uint32_t) to << ")");
  }

  if (auto p = 
      dynamic_cast<ObjectWithEventsInterface<Axis2>*>
      (&obj)) 
  {
    assert(trans_id > 0);
    /*LOGGER_DEBUG(logger, "update_events(" << trans_id 
      << ", " << to << ");");*/
    p->update_events(Axis2::self(), trans_id, to);
  }

  obj.state_changed
    (Axis2::self(), Axis2::self(), &obj, to);
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::compare_and_move(ObjectWithStatesInterface<Axis2>& obj, 
                   const RState<Axis>& from,
                   const RState<Axis>& to)
{
  LOGGER_TRACE(obj.logger(), "1");
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  TransitionId trans_id;
  auto& current = obj.current_state(Axis2::self());
  
  LOGGER_TRACE(obj.logger(), "2");
  uint32_t expected = current;
  if (STATE_IDX(expected) != STATE_IDX(from))
  {
  LOGGER_TRACE(obj.logger(), "2,5");
    return false;
  }

  LOGGER_TRACE(obj.logger(), "3");
  if (!(trans_id= StateMapInstance<Axis>::get_map()
        -> get_transition_id(from, to)))
    throw InvalidStateTransition(from, to);

  LOGGER_TRACE(obj.logger(), "4");
  if (!current.compare_exchange_strong(expected, to))
    return false;

  LOGGER_DEBUG(obj.logger(),
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << ::types::type<Axis>::name()
            << "/" << ::types::type<Axis2>::name()
            << ">\tcompare_and_move:\t "
            << UniversalState(expected).name()
            << std::hex << " (0x" << (uint32_t) from
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
  
  if (auto p = dynamic_cast<AbstractObjectWithEvents*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  obj.state_changed
    (Axis2::self(), Axis2::self(), &obj, to);

  return true;
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::compare_and_move
(ObjectWithStatesInterface<Axis2>& obj, 
 const std::set<RState<Axis>>& from_set,
 const RState<Axis>& to)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  TransitionId trans_id;
  uint32_t from;

  auto& current = obj.current_state(Axis2::self());
  do {
    from = current.load();

    // should we move?
    if (from_set.find(from) == from_set.end())
      return false;

    // check the from_set correctness
    if (!(trans_id= StateMapInstance<Axis>::get_map()
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from, to));

  LOGGER_DEBUG(obj.logger(),
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << ::types::type<Axis>::name()
            << "/" << ::types::type<Axis2>::name()
            << ">\tcompare_and_move:\t "
            << UniversalState(from).name()
            << std::hex << " (0x" << from
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
  if (auto p = dynamic_cast<RObjectWithEvents<Axis2>*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  obj.state_changed
    (Axis2::self(), Axis2::self(), &obj, to);

  return true;
}

template<class Axis, class Axis2>
auto RMixedAxis<Axis, Axis2>
//
::compare_and_move
  (ObjectWithStatesInterface<Axis2>& obj, 
   const std::map<const RState<Axis>, const RState<Axis>>& trs
   ) -> typename std::remove_reference
          <decltype(trs)>::type::const_iterator
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  TransitionId trans_id;
  uint32_t from;
  UniversalState to;
  typename std::remove_reference
    <decltype(trs)>::type::const_iterator ifrom;

  auto& current = obj.current_state(Axis2::self());
  do {
    from = current.load();

    // should we move?
    ifrom = trs.find(from);
    if (ifrom == trs.end())
      return ifrom;

    to = ifrom->second;
    assert(from == ifrom->first);

    // check the from_set correctness
    if (!(trans_id= StateMapInstance<Axis>::get_map()
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from, to));

  LOGGER_DEBUG(obj.logger(),
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << ::types::type<Axis>::name()
            << "/" << ::types::type<Axis2>::name()
            << ">\tcompare_and_move:\t "
            << UniversalState(from).name()
            << std::hex << " (0x" << from
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
  if (auto p = dynamic_cast<RObjectWithEvents<Axis2>*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  obj.state_changed
    (Axis2::self(), Axis2::self(), &obj, to);

  return ifrom;
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::neg_compare_and_move
(ObjectWithStatesInterface<Axis2>& obj, 
 const RState<Axis>& not_from,
 const RState<Axis>& to)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  TransitionId trans_id;
  uint32_t from_;
  UniversalState from;

  auto& current = obj.current_state(Axis2::self());
  do {
    from_ = current.load();
    from = UniversalState(from_);

    // should we move?
    if (STATE_IDX(from) == STATE_IDX(not_from))
      return false;

    if (!(trans_id= StateMapInstance<Axis>::get_map()
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from_, to));

  LOGGER_DEBUG(obj.logger(),
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << ::types::type<Axis>::name()
            << "/" << ::types::type<Axis2>::name()
            << ">\tneg_compare_and_move:\t "
            << UniversalState(from_).name()
            << std::hex << " (0x" << from_
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");

  if (auto p = dynamic_cast<RObjectWithEvents<Axis2>*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  obj.state_changed
    (Axis2::self(), Axis2::self(), &obj, to);

  return true;
}

template<class Axis, class Axis2>
uint32_t RMixedAxis<Axis, Axis2>
//
::state(const ObjectWithStatesInterface<Axis2>& obj)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  uint32_t us =
    const_cast<ObjectWithStatesInterface<Axis2>&>(obj)
    . current_state(Axis2::self()).load();
  //assert(STATE_MAP(us) == STATE_MAP(the_state));
  return us;
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::state_is (const ObjectWithStatesInterface<Axis2>& obj, 
            const RState<Axis>& st)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  return STATE_IDX(
    const_cast<ObjectWithStatesInterface<Axis2>&>(obj)
    . current_state(Axis2::self()).load()) 
    == STATE_IDX((const UniversalState&) st);
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::state_in(
  const ObjectWithStatesInterface<Axis2>& obj, 
  const std::initializer_list<RState<Axis>>& set )
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  const auto current = 
    const_cast<ObjectWithStatesInterface<Axis2>&> (obj)
    . current_state(Axis2::self()) . load();
  for (auto it = set.begin(); it != set.end(); it++)
  {
    if (STATE_IDX(current) == STATE_IDX(*it))
      return true;
  }
  return false;
}

template<class Axis, class Axis2>
template<template <class...> class Cont>
bool RMixedAxis<Axis, Axis2>
//
::state_in(
  const ObjectWithStatesInterface<Axis2>& obj, 
  const Cont<RState<Axis>>& set )
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  const auto current = 
    const_cast<ObjectWithStatesInterface<Axis2>&> (obj)
    . current_state(Axis2::self()) . load();
  for (auto it = set.begin(); it != set.end(); it++)
  {
    if (STATE_IDX(current) == STATE_IDX(*it))
      return true;
  }
  return false;
}

template<class Axis, class Axis2>
void RMixedAxis<Axis, Axis2>
//
::ensure_state
(const ObjectWithStatesInterface<Axis2>& obj, 
 const RState<Axis>& expected)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  if (!state_is(obj, expected)) {
    const auto current = 
      const_cast<ObjectWithStatesInterface<Axis2>&> (obj)
      . current_state(Axis2::self()) . load();

    InvalidState(current).raise(expected);
  }
}

template<class Axis, class Axis2>
void RMixedAxis<Axis, Axis2>
//
::ensure_state_in(
  const ObjectWithStatesInterface<Axis2>& obj, 
  const std::initializer_list<RState<Axis>>& set )
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  if (!state_in(obj, set)) {
    const auto current = 
      const_cast<ObjectWithStatesInterface<Axis2>&> (obj)
      . current_state(Axis2::self()) . load();

    throw ::types::exception(
      InvalidState(current),
      "expected a state from a set"
    );
  }
}

template<class Axis, class Axis2>
const StateMap* RMixedAxis<Axis, Axis2>
//
::state_map() 
{ 
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  return StateMapInstance<Axis>::get_map(); 
}

template<class Axis>
RState<Axis>::RState(::types::constexpr_string name)
: UniversalState
  (RAxis<Axis>::state_map() -> create_state(name))
{}

template<class Axis>
RState<Axis>::RState(uint32_t us)
: UniversalState(RAxis<Axis>::state_map(), us)
{}

template<class Axis>
RState<Axis>
//
::RState(const ObjectWithStatesInterface<Axis>& obj)
  : UniversalState
    (const_cast<ObjectWithStatesInterface<Axis>&>(obj)
     . current_state(Axis::self()))
{}

template<class Axis>
std::string RState<Axis>::name () const
{
  return RAxis<Axis>::state_map()
    -> get_state_name(*this);
}

template<class Axis>
template<class DerivedAxis>
RState<Axis>
//
::operator RState<DerivedAxis> () const
{
  static_assert(is_ancestor<Axis, DerivedAxis>(), 
                "This state mixing is invalid.");
  // change a state map index
  const StateMap* stateMap = 
    StateMapInstance<DerivedAxis>::get_map();
  assert(stateMap);
  /*if (!stateMap) 
    throw UnitializedAxis<DerivedAxis>();*/

  return RState<DerivedAxis>(
    STATE_IDX(the_state) 
    | (stateMap->numeric_id << STATE_MAP_SHIFT));
		
}

// <NB> no explicit default values
// (to prevent overwrite by static init code)
template<class Axis>
StateMap* StateMapInstance<Axis>::stateMap;
template<class Axis>
std::atomic<StateMapId>StateMapInstance<Axis>:: id;

//! Wait is_from_event then perform 
//! RMixedAxis<Axis,Axis2>::compare_and_move 
template<class T, class Axis2/* = typename T::axis*/>
void wait_and_move
  (T& obj, 
   const REvent<typename T::State::axis>& is_from_event,
   const RState<typename T::State::axis>& to,
   int wait_m)
{
  const auto from = is_from_event.to_state;

  do {
    CURR_WAIT_L(obj.logger(), is_from_event, wait_m);
  } 
  while (!compare_and_move<T, Axis2>(obj, from, to));
}

//! Wait is_from_event then perform 
//! RMixedAxis<Axis,Axis2>::compare_and_move
template<class T>
void wait_and_move
  (T& obj, 
   const std::set
     <RState<typename T::State::axis>>& from_set,
   const CompoundEvent& is_from_event,
   const RState<typename T::State::axis>& to,
   int wait_m/* = -1*/)
{
  do { 
    CURR_WAIT_L(obj.logger(), is_from_event, wait_m);
  } 
  while (!compare_and_move(obj, from_set, to));
}

}
#endif
