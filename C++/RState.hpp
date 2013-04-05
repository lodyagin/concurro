// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RState.h"
#include "Repository.hpp"
#include "RObjectWithStates.h"
#include <cstdatomic>

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

#if 0
template<class Axis>
RAxis<Axis>::RAxis
  (std::initializer_list<const char*> states,
	std::initializer_list<
	  std::pair<const char*, const char*>> transitions
  )
{
  StateMapPar<Axis> par(states, transitions);
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
#endif

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
  if (!stateMap->is_compatible(to))
	 throw IncompatibleMap();

  auto& current = obj.current_state();
  stateMap->check_transition(current, to); // A
  const auto from = current.exchange(to);  // B

  const TransitionId trans_id = 
	 stateMap->get_transition_id(from, to);

  if (trans_id == 0) {
	 // somebody changed state between A and B and a new
	 // state has not transition to a desired value. Make
	 // undo.
	 const auto old = current.exchange(from);
	 if (old != from)
		// unable to undo, sombody changed it again
		throw RaceConditionInStates(old, from, to);
	 else
		throw InvalidStateTransition(from, to);
  }

  if (auto p = dynamic_cast<RObjectWithEvents<Axis>*>
		(&obj)) {
	 assert(trans_id > 0);
	 p->update_events(trans_id, to);
  }

  LOGGER_DEBUG(obj.logger(), 
					"State changed from [" 
					<< stateMap->get_state_name(from)
					<< "] to [" 
					<< stateMap->get_state_name(to)
					<< "]");
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
