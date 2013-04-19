// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_OBJECTWITHSTATESINTERFACE_H_
#define CONCURRO_OBJECTWITHSTATESINTERFACE_H_

#include <log4cxx/logger.h>

template<class Axis> class RAxis;
template<class Axis> class RState;
template<class Axis> class REvent;

/// An interface which should be implemented in each
/// state-aware class.
template<class Axis>
class ObjectWithStatesInterface
{
  friend class RAxis<Axis>;
  friend class RState<Axis>;
  friend class REvent<Axis>;
public:
  typedef RState<Axis> State;

  virtual ~ObjectWithStatesInterface() {}

#if 0
  /// set state to the current state of the object
  virtual void state(State& state) const = 0;

  /// return bool if the object state is state
  virtual bool state_is(const State& state) const = 0;
#endif

  virtual log4cxx::LoggerPtr logger() const = 0;

  virtual std::string object_name() const
  {
	 return SFORMAT(typeid(this).name() 
						 << ":" << universal_id());
  }

  virtual std::string universal_id() const = 0;

protected:

  virtual std::atomic<uint32_t>& current_state() = 0;

#if 0
  /// Set the object state without transition cheking (do
  /// not use directly).
  virtual void set_state_internal 
	 (const State& state) = 0;
#endif
};

#endif
