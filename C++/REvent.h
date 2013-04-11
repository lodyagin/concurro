// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REVENT_H_
#define CONCURRO_REVENT_H_

#include "StateMap.h"
#include "RObjectWithStates.hpp"
#include "Event.h"

template<class Axis>
class REvent
: public Axis,
  protected UniversalEvent
{
public:
  //! Create a from->to event
  REvent(const char* from, const char* to);
  //! Create a *->to event
  REvent(const char* to);

  Event& event(RObjectWithEvents<Axis>&);

  //! Wait for the event on obj for max time msecs.
  //! \return false on timeout.
  bool wait(const RObjectWithEvents<Axis>& obj, 
            int time = -1) const;

  bool signalled(const RObjectWithEvents<Axis>& obj) const
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

#endif 
