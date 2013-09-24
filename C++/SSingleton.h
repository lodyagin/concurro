/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_SSINGLETON_H_
#define CONCURRO_SSINGLETON_H_

#include "Existent.h"
#include "SException.h"
#include <thread>

namespace curr {

/**
 * @defgroup singletons
 * Objects with counted number of instances.
 * @{
 */

DECLARE_AXIS(SingletonAxis, ExistenceAxis);

/** 
 * @class NotExistingSingleton
 * An exception raised when somebody
 * tries get an SSingleton instance when no one copy of
 * SSingleton<T> exists.
 */ 
class NotExistingSingleton;

/** 
 * @class MustBeSingleton
 * An exception raised when somebody tries create more
 * than once instance of SSingleton.
 */ 
class MustBeSingleton;

template<class T>
class SSingleton;

/**
 * A hook for check whether object is still singleton.
 */
template<class T>
class SingletonStateHook
{
  friend SSingleton<T>;
public:
  using Instance = ClassWithStates
    <T, ExistenceAxis, existent_class_initial_state, 
    SingletonStateHook<T>>;

  SingletonStateHook(Instance*);

  //! Disable change to ExistenceAxis exist_several state
  //! @exception MustBeSingleton
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);

protected:
  Instance* last_instance;
  static Instance* instance;
};

/**
 * A base class for classes that can have only one instance
 * parametrised by the actual singleton class, use as:
 * class MyClass : public SSingleton<MyClass>
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   not_exist [shape = doublecircle];
 *   start -> not_exist;
 *   not_exist -> preinc_exist_one
 *     [label="ctr:0"];
 *   preinc_exist_one -> exist_one
 *     [label="ctr:1"];
 *   exist_one -> preinc_exist_several
 *     [label="ctr:0"];
 *   preinc_exist_several -> exist_one
 *     [label="ctr (throw MustBeSingleton)"];
 *   preinc_exist_several -> exist_several
 *     [label="ctr:1"];
 *   exist_several -> preinc_exist_several
 *     [label="ctr:0"];
 *   exist_several -> predec_exist_several
 *     [label="dtr:0"];
 *   predec_exist_several -> exist_several
 *     [label="dtr:1"];
 *   predec_exist_several -> exist_one
 *     [label="dtr:1"];
 *   exist_one -> predec_exist_one
 *     [label="dtr:0"];
 *   predec_exist_one -> not_exist
 *     [label="dtr:1"];
 * }
 * @enddot
 *
 */
template<class T>
class SSingleton 
  : public Existent<T, SingletonStateHook<T>>
{
  friend SingletonStateHook<T>;
public:
  typedef Existent<T, SingletonStateHook<T>> Parent;

  //! One and only one class instance must be created with
  //! this function.
  //! @exception MustBeSingleton a copy of SSingleton<T>
  //! already exists.
  SSingleton();

  //! A deleted copy constructor
  SSingleton(const SSingleton&) = delete;

  //! The move constructor doesn't change existence.
  SSingleton(SSingleton&& s);

  virtual ~SSingleton();

  //! A deleted assignment operator.
  SSingleton& operator=(const SSingleton&) = delete;

  //! The move assignment is impossible unless lvalue and
  //! rvalue is the same object.
  SSingleton& operator=(SSingleton&& s) = delete;

  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  //!
  //! @exception NotExistingSingleton If no
  //! class is crated with SSingleton() raise exception.
  static T & instance();

  static bool isConstructed();

private:
  //! The singleton instance set by a hook (it differs
  //! with type from _instance)
  static typename SingletonStateHook<T>::Instance* 
    instance0;

  //! The actual singleton instance. It is updated from
  //! instance0 in instance() call.
  static T* _instance;

  void set_instance
    (typename SingletonStateHook<T>::Instance* inst)
  { 
     instance0 = inst; 
  }
};

/**
 * Extends SSingleton to allow auto-construct it by the
 * RAutoSingleton::instance() call.
 */
template<class T>
class SAutoSingleton : public SSingleton<T>
{
public:
  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  static T & instance ();
};

template<>
class SAutoSingleton<
  RMixedAxis<ExistenceAxis, ExistenceAxis>> 
{
public:
  typedef RMixedAxis<ExistenceAxis, ExistenceAxis> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<>
class SAutoSingleton<
  RMixedAxis<SingletonAxis, SingletonAxis>> 
{
public:
  typedef RMixedAxis<SingletonAxis, SingletonAxis> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<>
class SAutoSingleton<StateMapRepository> 
{
public:
  typedef StateMapRepository T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<class Thread>
class RThreadRepository;

#if 0
template<class Thread>
class SAutoSingleton<RThreadRepository<Thread>> 
{
public:
  typedef RThreadRepository<Thread> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};
#else
template<class SystemThread>
class RThread;

template<>
class SAutoSingleton<
  RThreadRepository<RThread<std::thread>>> 
{
public:
  typedef RThreadRepository<RThread<std::thread>> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<class Axis>
class StateMapInstance;

template<>
class SAutoSingleton<StateMapInstance<StateAxis>>
{
public:
  typedef StateMapInstance<StateAxis> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<>
class SAutoSingleton<StateMapInstance<ExistenceAxis>>
{
public:
  typedef StateMapInstance<ExistenceAxis> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

template<>
class SAutoSingleton<StateMapInstance<SingletonAxis>>
{
public:
  typedef StateMapInstance<SingletonAxis> T;
  friend T;

  SAutoSingleton(const SAutoSingleton&) = delete;
  virtual ~SAutoSingleton() {}
  SAutoSingleton& operator=(const SAutoSingleton&) =delete;

  static T& instance();

private:
  SAutoSingleton() {}
};

#endif

//! @}

}
#endif
