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
#include <atomic>
#include <thread>

namespace curr {

//! @defgroup Singletons
//! @{

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

struct SingletonStateHook
{
  //! Disable change to ExistenceAxis exist_several state
  //! @exception MustBeSingleton
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);
};

/**
 * Base class for classes that can have only one instance
 * parametrised by the actual singleton class, use as:
 * class MyClass : public SSingleton<MyClass>
 */
template<class T>
class SSingleton : public Existent<T, SingletonStateHook>
{
public:
  //! One and only one class instance must be created with
  //! this function.
  //! @exception MustBeSingleton a copy of SSingleton<T>
  //! already exists.
  SSingleton();

  //! A deleted copy constructor
  SSingleton(const SSingleton&) = delete;

  //! The move constructor doesn't change existence.
  SSingleton(SSingleton&&) {}

  virtual ~SSingleton();

  //! A deleted assignment operator.
  SSingleton& operator=(const SSingleton&) = delete;

  //! The move assignment doesn't change existence.
  SSingleton& operator=(SSingleton&&) {}

  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  //!
  //! @exception NotExistingSingleton If no
  //! class is crated with SSingleton() raise exception.
  static T & instance();

  static bool isConstructed ()
  {
     return _instance;
  }

private:
  static std::atomic<T*> _instance;
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
