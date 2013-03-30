// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "Repository.h"

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
		  . get_object_by_id(typeid(Axis).name());
	 }
	 catch(const StateMapRepository::NoSuchId&)
	 {
		stateMap = StateMapRepository::instance()
		  . create_object(par);
	 }
  }

  *((UniversalState*) this) = stateMap->create_state(name);
}

template<class Axis>
RState<Axis>::RState(const UniversalState& us)
  : UniversalState(us)
{
}


template<class Axis>
void RState<Axis>
//
::check_moving_to(const ObjectWithStatesInterface<Axis>& obj, const RState& to)
{
	RState from = to;
	obj.state(from);
	from.stateMap->check_transition (from, to);
}

template<class Axis>
void RState<Axis>
//
::move_to(ObjectWithStatesInterface<Axis>& obj, const RState& to)
{
	check_moving_to (obj, to);
	obj.set_state_internal (to);
}

template<class Axis>
bool RState<Axis>
//
::state_is (const ObjectWithStatesInterface<Axis>& obj, const RState& st)
{
#if 0
	RState current = st;
	obj.state (current);
	return current == st;
#else
	return obj.state_is(st);
#endif
}
