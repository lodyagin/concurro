// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_OBJECTWITHSTATESINTERFACE_H_
#define CONCURRO_OBJECTWITHSTATESINTERFACE_H_

#include <log4cxx/logger.h>
#include "StateMap.h"

template<class Axis1, class Axis2> class RMixedAxis;
template<class Axis> class RState;
template<class Axis1, class Axis2> class RMixedEvent;

class AbstractObjectWithStates
{
public:
  virtual ~AbstractObjectWithStates() {}

  // Retrieve a main axis
  //virtual StateAxis& get_axis() const = 0;

  //! the "update parent" callback on state changing in
  //! the `object'.
  virtual void state_changed
    (StateAxis& ax, AbstractObjectWithStates* object) = 0;

  //! Terminal state means 
  //! 1) no more state activity;
  //! 2) the object can be deleted (there are no more
  //! dependencies on it).
  virtual CompoundEvent is_terminal_state() const = 0;

protected:
  virtual std::atomic<uint32_t>& 
	 current_state(const StateAxis&) = 0;

  virtual const std::atomic<uint32_t>& 
	 current_state(const StateAxis&) const = 0;
};

class RObjectWithStatesBase;

/// An interface which should be implemented in each
/// state-aware class.
template<class Axis>
class ObjectWithStatesInterface
: public virtual AbstractObjectWithStates
{
  template<class Axis1, class Axis2> 
	 friend class RMixedAxis;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
public:
  typedef Axis axis;
  typedef RState<Axis> State;

  virtual ~ObjectWithStatesInterface() {}

  virtual log4cxx::LoggerPtr logger() const = 0;

  virtual std::string object_name() const
  {
	 return SFORMAT(typeid(this).name() 
                        << ":" << universal_id());
  }

  /*StateAxis& get_axis() const override 
  { 
    return Axis::self(); 
    }*/

  virtual void state_changed
    (StateAxis& ax, AbstractObjectWithStates* object) = 0;

  virtual std::string universal_id() const = 0;
};

class AbstractObjectWithEvents
{
  template<class Axis1, class Axis2> 
	 friend class RStateSplitter;
public:
  virtual ~AbstractObjectWithEvents() {}

  //! Update events due to trans_id to
  virtual void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) = 0;
#if 0
unable to have events from different axes in one class
protected:
  //! Query an event object by UniversalEvent. 
  virtual Event get_event(const UniversalEvent& ue) = 0;

  //! Query an event object by UniversalEvent. 
  virtual Event get_event
	 (const UniversalEvent& ue) const = 0;

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  virtual Event create_event
	 (const UniversalEvent&) const = 0;

#endif
};

template<class Axis>
class ObjectWithEventsInterface
: public virtual AbstractObjectWithEvents
{
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
  friend class RMixedAxis;
public:
  virtual ~ObjectWithEventsInterface() {}

protected:
#if 0
  //! Query an event object by UniversalEvent. 
  virtual Event get_event(const UniversalEvent& ue) = 0;

  //! Query an event object by UniversalEvent. 
  virtual Event get_event
     (const UniversalEvent& ue) const = 0;
#endif

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  virtual CompoundEvent create_event
     (const UniversalEvent&) const = 0;

  //! Update events due to trans_id to
  virtual void update_events
     (StateAxis& ax, 
      TransitionId trans_id, 
      uint32_t to) = 0;
};

#endif
