// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSTATE_HPP_
#define CONCURRO_RSTATE_HPP_

#include "RState.h"
#include "RThread.h"
#include "Repository.hpp"
#include "RObjectWithStates.hpp"
#include <atomic>

template<class Axis>
StateMap* StateMapInstance<Axis>::stateMap = nullptr;

template<class Axis>
StateMapId StateMapInstance<Axis>::init()
{
  if (!stateMap) {
    StateMapInstance<typename Axis::Parent>::init();
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
  }
  assert(stateMap);
  return stateMap->numeric_id;
}

template<class Axis, class Axis2>
RMixedAxis<Axis, Axis2>
//
::RMixedAxis(/*const StateMapPar<Axis>& par*/)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  StateMapInstance<Axis>::init();
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
  StateMapInstance<Axis>::stateMap
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
  auto logger = obj.logger(); 
  // <NB> get the logger before possible destruction of
  // obj (i.e. as a result of move to a terminal state)

  do {
    from = current.load();
    if (!(trans_id=
          StateMapInstance<Axis>::stateMap
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);
  } while (!current.compare_exchange_strong(from, to));

  obj.state_changed(Axis2::self(), Axis2::self(), &obj);

  if (auto p = 
      dynamic_cast<ObjectWithEventsInterface<Axis2>*>
      (&obj)) 
  {
    assert(trans_id > 0);
    /*LOGGER_DEBUG(logger, "update_events(" << trans_id 
      << ", " << to << ");");*/
    p->update_events(Axis2::self(), trans_id, to);
  }

  LOGGER_DEBUG(logger, 
               "thread " 
               << RThread<std::thread>::current_pretty_id()
               << ">\t object " << obj.object_name()
               << ">\t axis " << typeid(Axis).name()
               << ">\t " << UniversalState(from).name()
               << std::hex << " (0x" << from
               << ") -> " << to.name() << " (0x" 
               << (uint32_t) to << ")");
}

template<class Axis, class Axis2>
bool RMixedAxis<Axis, Axis2>
//
::compare_and_move(ObjectWithStatesInterface<Axis2>& obj, 
                   const RState<Axis>& from,
                   const RState<Axis>& to)
{
  static_assert(is_ancestor<Axis2, Axis>(), 
                "This state mixing is invalid.");
  TransitionId trans_id;
  auto& current = obj.current_state(Axis2::self());
  
  if (STATE_IDX(current) != STATE_IDX(from))
    return false;

  if (!(trans_id= StateMapInstance<Axis>::stateMap
        -> get_transition_id(from, to)))
    throw InvalidStateTransition(from, to);

  uint32_t expected = current;
  if (!current.compare_exchange_strong(expected, to))
    return false;

  obj.state_changed(Axis2::self(), Axis2::self(), &obj);
  
  if (auto p = dynamic_cast<AbstractObjectWithEvents*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  LOG_DEBUG(log,
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << typeid(Axis).name()
            << ">\t " << UniversalState(from).name()
            << std::hex << " (0x" << (uint32_t) from
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
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
    if (!(trans_id= StateMapInstance<Axis>::stateMap
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from, to));

  obj.state_changed(Axis2::self(), Axis2::self(), &obj);

  if (auto p = dynamic_cast<RObjectWithEvents<Axis2>*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  LOG_DEBUG(log,
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << typeid(Axis).name()
            << ">\t " << UniversalState(from).name()
            << std::hex << " (0x" << from
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
  return true;
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

    if (!(trans_id= StateMapInstance<Axis>::stateMap
          -> get_transition_id(from, to)))
      throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from_, to));

  obj.state_changed(Axis2::self(), Axis2::self(), &obj);

  if (auto p = dynamic_cast<RObjectWithEvents<Axis2>*>
      (&obj)) {
    assert(trans_id > 0);
    p->update_events(Axis2::self(), trans_id, to);
  }

  LOG_DEBUG(log,
            "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t object " << obj.object_name()
            << ">\t axis " << typeid(Axis).name()
            << ">\t " << UniversalState(from).name()
            << std::hex << " (0x" << from_
            << ") -> " << to.name() << " (0x" 
            << (uint32_t) to << ")");
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
bool RMixedAxis<Axis, Axis2>
//
::state_in(
  const ObjectWithStatesInterface<Axis2>& obj, 
  const std::set<RState<Axis>>& set )
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

    throw InvalidState(current, expected);
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

    throw InvalidState(current, 
                       "expected a state from a set");
  }
}

template<class Axis>
RState<Axis>::RState (const char* name)
: UniversalState
(RAxis<Axis>::instance().state_map()
 -> create_state(name))
{}

template<class Axis>
RState<Axis>::RState(uint32_t us)
: UniversalState(RAxis<Axis>::instance().state_map(), us)
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
  return RAxis<Axis>::instance().state_map()
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
    StateMapInstance<DerivedAxis>::stateMap;
  if (!stateMap) 
    throw UnitializedAxis<DerivedAxis>();

  return RState<DerivedAxis>(
    STATE_IDX(the_state) 
    | (stateMap->numeric_id << STATE_MAP_SHIFT));
		
}


#endif
