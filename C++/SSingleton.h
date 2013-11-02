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
#include "ConstructibleObject.h"
#include "Event.h"
#include "Logging.h"
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

template<class T, int wait_m>
class SSingleton;

/**
 * A hook for check whether object is still singleton.
 */
template<class T, int wait_m>
class SingletonStateHook
{
  friend SSingleton<T, wait_m>;
public:
  //! Disable change to ExistenceAxis exist_several state
  //! @exception MustBeSingleton
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);
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
template<class T, int wait_m = 1000>
class SSingleton 
  : public virtual Existent
      <T, SingletonStateHook<T, wait_m>>,
    public virtual ConstructibleObject
{
  friend SingletonStateHook<T, wait_m>;
public:
  typedef Existent<T, SingletonStateHook<T, wait_m>> 
    Parent;
  typedef ConstructibleObject ObjParent;
  typedef SSingleton<T, wait_m> This;

  //! One and only one class instance must be created with
  //! this function.
  //! @exception MustBeSingleton a copy of SSingleton<T,
  //! wait_m> already exists.
  SSingleton();

  //! A deleted copy constructor
  SSingleton(const SSingleton&) = delete;

  //! The move constructor doesn't change existence.
  SSingleton(SSingleton&& s) = delete;

  virtual ~SSingleton();

  //! A deleted assignment operator.
  SSingleton& operator=(const SSingleton&) = delete;

  //! The move assignment is impossible unless lvalue and
  //! rvalue is the same object.
  SSingleton& operator=(SSingleton&& s) = delete;

  CompoundEvent is_terminal_state() const override
  {
    return E(complete_construction);
  }

  void complete_construction() override;

  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  //!
  //! @exception NotExistingSingleton If no
  //! class is crated with SSingleton() raise exception.
  static T & instance();

  //! It is the same as !state_is(*this, in_construction)
  bool isConstructed();

protected:
  //! It is a statically accessible analog of
  //! the complete_construction event.
  static Event is_complete()
  {
    static Event is_complete_event
      (SFORMAT(typeid(SSingleton<T, wait_m>).name()
               << ":is_complete()::is_complete_event"), 
       true, false);
    // prevent infinit loop in RThreadRepository::current
    is_complete_event.log_params().wait = false;
    return is_complete_event;
  }

private:
  typedef Logger<SSingleton<T, wait_m>> log;

  //! The actual singleton instance. It is updated from
  //! instance0 in instance() call.
  static T* _instance;
};

/**
 * Extends SSingleton to allow auto-construct it by the
 * RAutoSingleton::instance() call.
 * \tparam wait_m The default construction wait timeout.
 */
template<class T, int wait_m = 1000>
class SAutoSingleton : public SSingleton<T, wait_m>
{
public:
  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  static T & instance ();

protected:
  //! Create the object if doesn't exist
  static void construct_once();

private:
  typedef Logger<SAutoSingleton<T, wait_m>> log;
};

//! @}

}
#endif
