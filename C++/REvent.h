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

#if 0
class REventInterface
{
public:
#if 0
  //! Wait for the event on obj for max time msecs.
  //! \return false on timeout.
  virtual bool wait(int time = -1) const = 0;

  //! Is it already signalled?
  virtual bool signalled() const = 0;
#endif

  virtual ~REventInterface() {}

  virtual operator Event& () = 0;
  virtual operator const Event& () const = 0;
};
#endif

template<class Axis>
class REvent
: public Axis,
  public UniversalEvent
{
public:
  //! Create a from->to event
  REvent(RObjectWithEvents<Axis>* obj_ptr, 
			const char* from, const char* to);

  //! Create a *->to event
  REvent(RObjectWithEvents<Axis>* obj_ptr, 
			const char* to);

  bool wait(int time = -1) const
  { 
	 return static_cast<const Event&>(*this).wait(time); 
  }

  bool signalled() const
  {
	 return static_cast<const Event&>(*this).signalled();
  }

  operator Event& () 
  { assert(evt); return *evt; }

  operator const Event& () const 
  { assert(evt); return *evt; }

protected:
  //RObjectWithEvents<Axis>* obj;
  Event* evt;
};

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
