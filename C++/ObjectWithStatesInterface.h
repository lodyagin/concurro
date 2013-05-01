// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_OBJECTWITHSTATESINTERFACE_H_
#define CONCURRO_OBJECTWITHSTATESINTERFACE_H_

#include <log4cxx/logger.h>

template<class Axis1, class Axis2> class RMixedAxis;
template<class Axis> class RState;
template<class Axis1, class Axis2> class RMixedEvent;

/// An interface which should be implemented in each
/// state-aware class.
template<class Axis>
class ObjectWithStatesInterface
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

  virtual std::string universal_id() const = 0;

protected:

  virtual std::atomic<uint32_t>& current_state() = 0;
  virtual const std::atomic<uint32_t>& 
	 current_state() const = 0;
};

template<class Axis>
class ObjectWithEventsInterface
{
  template<class Axis1, class Axis2> 
	 friend class RMixedEvent;
  friend class RState<Axis>;
  template<class Axis1, class Axis2> 
	 friend class RMixedAxis;
protected:
  //! Query an event object by UniversalEvent. 
  virtual Event get_event(const UniversalEvent& ue) = 0;

  //! Query an event object by UniversalEvent. 
  virtual const Event get_event
	 (const UniversalEvent& ue) const = 0;

  //! Register a new event in the map if it doesn't
  //! exists. In any case return the event.
  virtual Event create_event
	 (const UniversalEvent&) const = 0;

  //! Update events due to trans_id to
  virtual void update_events
	 (TransitionId trans_id, uint32_t to) = 0;
};

#endif
