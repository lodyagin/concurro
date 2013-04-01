// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_H
#define CONCURRO_REVENT_H

#include "StateMap.h"
#include "RObjectWithStates.h"
#include "Event.h"

#if 0
class ToEvent
{
public:
  StateIdx state_idx;

protected:
  ToEvent(StateIdx state)
	 : state_idx(state) {}
};
#endif

template<class Axis>
class REvent
: public Axis,
  protected UniversalEvent
  //public SAutoSingleton<REvent<Object, Axis>>
{
public:
  //! Create a from->to event
  REvent(const char* from, const char* to);
  //! Create a *->to event
  REvent(const char* to);

  Event& event(RObjectWithEvents<Axis>&);

  bool wait(RObjectWithEvents<Axis>& obj, int time 
				= std::numeric_limits<uint64_t>::max())
  {
	 return event(obj).wait(time);
  }

  bool signalled(RObjectWithEvents<Axis>& obj) const
  {
	 return const_cast<REvent<Axis>*>(this)
		-> event(obj).signalled();
  }
};

#if 0
//! Create an event of moving Obj from `before' state to
//! `after' state.
const UniversalEvent& operator / 
  (const UniversalState& before, 
	const UniversalState& after);

//! Change a state according to an event
const UniversalState& operator + 
  (const UniversalState& state, 
	const UniversalEvent& event);
#endif

//size_t waitMultiple( HANDLE *, size_t count );

// include shutdown event also
//size_t waitMultipleSD( HANDLE *, size_t count );

#include "REvent.hpp"

#endif 
