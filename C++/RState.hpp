// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "Repository.h"
#include <cstdatomic>

template<class Axis>
StateMap* RState<Axis>::stateMap = 0;

template<class Axis>
RState<Axis>::RState (const StateMapPar<Axis>& par,
							 const char* name)
{
  assert (name);

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

  the_state = stateMap->create_state(name);
}

template<class Axis>
RState<Axis>::RState(uint32_t us)
{
  set_by_universal(us);
}

template<class Axis>
RState<Axis>
//
::RState(const ObjectWithStatesInterface<Axis>& obj)
{
  const uint32_t us = 
	 const_cast<ObjectWithStatesInterface<Axis>&>(obj)
	 . current_state();
  set_by_universal(us);
}

template<class Axis>
void RState<Axis>
//
::set_by_universal (uint32_t us)
{
  assert(stateMap);
  if (!stateMap->is_compatible(us))
	 throw IncompatibleMap();

  the_state = us;
}

template<class Axis>
void RState<Axis>
//
::check_moving_to(
  const ObjectWithStatesInterface<Axis>& obj, 
  const RState& to)
{
  const uint32_t from = 
	 const_cast<ObjectWithStatesInterface<Axis>&> (obj)
	 . current_state().load();
  stateMap->check_transition (from, to);
}

template<class Axis>
void RState<Axis>
//
::move_to(ObjectWithStatesInterface<Axis>& obj, 
			 const RState& to)
{
  if (!stateMap->is_compatible(to))
	 throw IncompatibleMap();

#ifdef STATE_LOCKING
  auto& current = obj.current_state();
  const auto from = current;
  {
	 RLOCK(lock);
	 from.state_map->check_transition(current, to);
	 current = to;
  }
#else
  auto& current = obj.current_state();
  stateMap->check_transition(current, to);
  const auto from = current.exchange(to);
  try {
	 stateMap->check_transition(from, to);
  }
  catch (const InvalidStateTransition&) {
	 const auto old = current.exchange(from);
	 if (old != from)
		throw RaceConditionInStates(old, from, to);
	 //throw;
  }
#endif
  LOGGER_DEBUG(obj.logger(), 
					"State changed from [" 
					<< stateMap->get_state_name(from)
					<< "] to [" 
					<< stateMap->get_state_name(to)
					<< "]");
}

template<class Axis>
bool RState<Axis>
//
::state_is (const ObjectWithStatesInterface<Axis>& obj, 
				const RState& st)
{
  return
	 const_cast<ObjectWithStatesInterface<Axis>&>(obj)
	 . current_state().load() == /*(UniversalState)*/ st;
}

template<class Axis>
bool RState<Axis>
//
::state_in(
  const ObjectWithStatesInterface<Axis>& obj, 
  const std::initializer_list<RState>& set )
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
uint32_t RState<Axis>
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
std::ostream&
operator<< (std::ostream& out, const RState<Axis>& st)
{
  out << st.name();
  return out;
}
