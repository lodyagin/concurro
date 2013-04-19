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
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif

template<class Axis>
StateMap* RAxis<Axis>::stateMap = 0;

template<class Axis>
RAxis<Axis>::RAxis(const StateMapPar<Axis>& par)
{
  if (!stateMap) {
	 try {
		stateMap = StateMapRepository::instance()
		  . get_map_for_axis(typeid(Axis));
	 }
	 catch(const StateMapRepository::NoSuchId&)
	 {
		stateMap = StateMapRepository::instance()
		  . create_object(par);
	 }
  }
}

template<class Axis>
void RAxis<Axis>
//
::check_moving_to(
  const ObjectWithStatesInterface<Axis>& obj, 
  const RState<Axis>& to)
{
  const uint32_t from = 
	 const_cast<ObjectWithStatesInterface<Axis>&> (obj)
	 . current_state().load();
  stateMap->check_transition (from, to);
}

template<class Axis>
void RAxis<Axis>
//
::move_to(ObjectWithStatesInterface<Axis>& obj, 
			 const RState<Axis>& to)
{
  uint32_t from;
  TransitionId trans_id;
  auto& current = obj.current_state();
  auto logger = obj.logger(); 
  // <NB> get the logger before possible destruction of
  // obj (i.e. as a result of move to a terminal state)

  do {
	 from = current.load();
	 if (!(trans_id= stateMap->get_transition_id(from,to)))
		throw InvalidStateTransition(from, to);
  } while (!current.compare_exchange_strong(from, to));

  if (auto p = dynamic_cast<RObjectWithEvents<Axis>*>
		(&obj)) {
	 assert(trans_id > 0);
	 /*LOGGER_DEBUG(logger, "update_events(" << trans_id 
		<< ", " << to << ");");*/
	 p->update_events(trans_id, to);
  }

  LOGGER_DEBUG(logger, 
					"thread " 
					<< RThread<std::thread>::current_pretty_id()
					<< ">\t object " << obj.object_name()
					<< ">\t " //state is changed from [" 
					<< stateMap->get_state_name(from)
					<< " -> " //"] to [" 
					<< stateMap->get_state_name(to));
					//<< "]");
}

template<class Axis>
bool RAxis<Axis>
//
::compare_and_move(ObjectWithStatesInterface<Axis>& obj, 
						 const RState<Axis>& from,
						 const RState<Axis>& to)
{
  TransitionId trans_id;
  auto& current = obj.current_state();
  
  if (current != from)
	 return false;

  if (!(trans_id= stateMap->get_transition_id(from,to)))
	 throw InvalidStateTransition(from, to);

  uint32_t expected = from;
  if (!current.compare_exchange_strong(expected, to))
	 return false;
  
  if (auto p = dynamic_cast<RObjectWithEvents<Axis>*>
		(&obj)) {
	 assert(trans_id > 0);
	 p->update_events(trans_id, to);
  }

  LOG_DEBUG(log, 
					"thread " 
					<< RThread<std::thread>::current_pretty_id()
					<< ">\t object " << obj.object_name()
					<< ">\t " //state is changed from [" 
					<< stateMap->get_state_name(from)
					<< " -> " //"] to [" 
					<< stateMap->get_state_name(to));
					//<< "]");
  return true;
}

template<class Axis>
bool RAxis<Axis>
//
::compare_and_move
  (ObjectWithStatesInterface<Axis>& obj, 
	const std::set<RState<Axis>>& from_set,
	const RState<Axis>& to)
{
  TransitionId trans_id;
  uint32_t from;

  auto& current = obj.current_state();
  do {
	 from = current.load();

	 // should we move?
	 if (from_set.find(from) == from_set.end())
		return false;

	 // check the from_set correctness
	 if (!(trans_id= stateMap->get_transition_id(from,to)))
		throw InvalidStateTransition(from, to);

  } while(!current.compare_exchange_strong(from, to));

  if (auto p = dynamic_cast<RObjectWithEvents<Axis>*>
		(&obj)) {
	 assert(trans_id > 0);
	 p->update_events(trans_id, to);
  }

  LOG_DEBUG(log, 
					"thread " 
					<< RThread<std::thread>::current_pretty_id()
					<< ">\t object " << obj.object_name()
					<< ">\t " //state is changed from [" 
					<< stateMap->get_state_name(from)
					<< " -> " //"] to [" 
					<< stateMap->get_state_name(to));
					//<< "]");
  return true;
}

template<class Axis>
uint32_t RAxis<Axis>
//
::state(const ObjectWithStatesInterface<Axis>& obj)
{
  uint32_t us =
	 const_cast<ObjectWithStatesInterface<Axis>&>(obj)
	 . current_state().load();
  //assert(STATE_MAP(us) == STATE_MAP(the_state));
  return us;
}

template<class Axis>
bool RAxis<Axis>
//
::state_is (const ObjectWithStatesInterface<Axis>& obj, 
				const RState<Axis>& st)
{
  return
	 const_cast<ObjectWithStatesInterface<Axis>&>(obj)
	 . current_state().load() == /*(UniversalState)*/ st;
}

template<class Axis>
bool RAxis<Axis>
//
::state_in(
  const ObjectWithStatesInterface<Axis>& obj, 
  const std::initializer_list<RState<Axis>>& set )
{
  const auto current = 
	 const_cast<ObjectWithStatesInterface<Axis>&> (obj)
	 . current_state() . load();
  for (auto it = set.begin(); it != set.end(); it++)
  {
	 if (current == *it)
		return true;
  }
  return false;
}

template<class Axis>
void RAxis<Axis>
//
::ensure_state
  (const ObjectWithStatesInterface<Axis>& obj, 
	const RState<Axis>& expected)
{
  if (!state_is(obj, expected)) {
	 const auto current = 
		const_cast<ObjectWithStatesInterface<Axis>&> (obj)
		. current_state() . load();

	 throw InvalidState(current, expected);
  }
}

template<class Axis>
RState<Axis>::RState (const char* name)
  : UniversalState
	 (RAxis<Axis>::instance().state_map()
	  . create_state(name))
{}

template<class Axis>
RState<Axis>::RState(uint32_t us)
  : UniversalState(us)
{
  // ensure the new state is from Axis
  RAxis<Axis>::instance().state_map().is_compatible(us);
}

template<class Axis>
RState<Axis>
//
::RState(const ObjectWithStatesInterface<Axis>& obj)
  : UniversalState
  (const_cast<ObjectWithStatesInterface<Axis>&>(obj)
	. current_state())
{}

template<class Axis>
std::string RState<Axis>::name () const
{
  return RAxis<Axis>::instance().state_map()
	 .get_state_name(*this);
}

template<class Axis>
std::ostream&
operator<< (std::ostream& out, const RState<Axis>& st)
{
  out << st.name();
  return out;
}

#endif
