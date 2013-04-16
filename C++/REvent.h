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

class REventInterface
{
public:
  //! Wait for the event on obj for max time msecs.
  //! \return false on timeout.
  virtual bool wait(int time = -1) const = 0;

  //! Is it already signalled?
  virtual bool signalled() const = 0;
};

template<class Axis>
class REvent
: public REventInterface,
  public Axis,
  protected UniversalEvent
{
public:
  //! Create a from->to event
  REvent(RObjectWithEvents<Axis>* obj_ptr, 
			const char* from, const char* to);

  //! Create a *->to event
  REvent(RObjectWithEvents<Axis>* obj_ptr, 
			const char* to);

  bool wait(int time = -1) const;
  bool signalled() const;

protected:
  RObjectWithEvents<Axis>* obj;
};

class RCompoundEvent
{
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

#define DECLARE_EVENT(axis, event) \
protected: \
  REvent<axis> is_ ## event ## _event; \
public: \
  const REvent<axis>& is_ ## event () \
  { return is_ ## event ## _event; } \
private:

#if 0
#define DEFINE_EVENT(class_, axis, event) \
  REvent<axis> class_::is_ ## event ## _event(#event);
#else
#define CONSTRUCT_EVENT(event)		\
  is_ ## event ## _event(this, #event)
#endif

#endif 
