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
#include <set>

template<class Axis, class Axis2>
class RMixedEvent
: public Axis,
  public UniversalEvent,
  public CompoundEvent
{
public:
  //! Create a from->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
				  const char* from, const char* to);

  //! Create a *->to event
  RMixedEvent(ObjectWithEventsInterface<Axis2>* obj_ptr, 
				  const char* to);
private:
  typedef Logger<LOG::Events> log;
};

template<class Axis>
using REvent = RMixedEvent<Axis, Axis>;

#define A_DECLARE_EVENT(axis_, axis_2, event)		\
protected: \
RMixedEvent<axis_, axis_2> is_ ## event ## _event;	\
public: \
  const RMixedEvent<axis_, axis_2>& is_ ## event ()	\
  { return is_ ## event ## _event; } \
private:

#define DECLARE_EVENT(axis_, event) \
  A_DECLARE_EVENT(axis_, axis_, event)

#define CONSTRUCT_EVENT(event)		\
  is_ ## event ## _event(this, #event)

#endif 
