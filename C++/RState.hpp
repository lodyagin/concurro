// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "Repository.h"
#include "RObjectWithStates.h"
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

inline std::ostream&
operator<< (std::ostream& out, const UniversalState& st)
{
  out << StateMapRepository::instance()
	 . get_state_name(st.the_state);
  return out;
}

template<class Axis>
std::ostream&
operator<< (std::ostream& out, const RState<Axis>& st)
{
  out << st.name();
  return out;
}
