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

#ifndef CONCURRO_SSINGLETON_HPP_
#define CONCURRO_SSINGLETON_HPP_

#include <list>
#include <boost/lockfree/stack.hpp>
#include "SSingleton.h"
#include "RMutex.h"
#include "Logging.h"
#include "SCheck.h"
#include "Existent.hpp"

namespace curr {

//! Exception: somebody tries to get an
//! SSingleton::instance() when no one is available
class NotExistingSingleton : public curr::SException
{
public:
  NotExistingSingleton()
    : curr::SException("Somebody tries to get "
                       "an SSingleton::instance() when "
                       "no one is available") {}
};

//! Exception: somebody tries to create more than one
//! SSingleton instance
class MustBeSingleton : public curr::SException
{
public:
  MustBeSingleton()
    : curr::SException(
      "Somebody tries to create more than one "
      "SSingleton instance") {}
};

// SingletonStateHook

template<class T, int wait_m>
void SingletonStateHook<T, wait_m>::operator() 
  (AbstractObjectWithStates* object,
   const StateAxis&,
   const UniversalState& new_state)
{
  // Unable to cast to Existent because it is not
  // constructed yet
  auto* obj_ptr = dynamic_cast
    <ObjectWithStatesInterface<ExistenceAxis>*>(object);

  SCHECK(obj_ptr);
  auto& obj = *obj_ptr;
  const RState<ExistenceAxis> newst(new_state);

  if (newst == SSingleton<T, wait_m>::TheClass
        ::preinc_exist_severalFun())
      // it is obligatory to check reason of enter to
      // disable intercept preinc_exist_several set by
      // others.
  {
    if(
      RMixedAxis<SingletonAxis, ExistenceAxis>
      ::compare_and_move(
        obj,
        SSingleton<T, wait_m>::TheClass::preinc_exist_severalFun(),
        // <NB> prior obj_count++
        SSingleton<T, wait_m>::TheClass::exist_oneFun()
        // <NB> the singleton constructor will be failed
        ))
      THROW_EXCEPTION(MustBeSingleton);
    else
      THROW_PROGRAM_ERROR; // check the state design if
                           // you get it
  }
}

// SSingleton

//FIXME concurrency problems still here

template<class T, int wait_m>
T* SSingleton<T, wait_m>::_instance; // <NB> no explicit init

template<class T, int wait_m>
SSingleton<T, wait_m>::SSingleton()
{
  assert(this->get_obj_count() == 1);
}

#if 0
template<class T, int wait_m>
SSingleton<T, wait_m>::SSingleton(SSingleton&& s)
 : Parent(std::move(s))
{
  assert(this->get_obj_count() == 1);
  set_instance(this);
  _instance = nullptr;
  RMixedAxis<SingletonAxis, ExistenceAxis>::move_to
    (this->theClass, exist_oneFun());
  assert(this->get_obj_count() == 1);
}
#endif

template<class T, int wait_m>
SSingleton<T, wait_m>::~SSingleton()
{
  if (!this->paired)
  {
    // do not call it for moved-from objects (that have
    // paired pointed to a new moved-to instance)

    SCHECK(this->get_obj_count() == 1);
    is_complete().reset();
    _instance = nullptr;
  }
}

template<class T, int wait_m>
void SSingleton<T, wait_m>::complete_construction()
{
  if ((_instance = dynamic_cast<T*>(this))) 
  { // wait the call from T::T
    ObjParent::complete_construction();
    is_complete().set();
  }
}

template<class T, int wait_m>
inline T & SSingleton<T, wait_m>::instance()
{
  // If another thread starts creating T we must wait the
  // completion of the creation
  try {
    CURR_WAIT(This::is_complete(), wait_m);
  }
  catch (const EventWaitingTimedOut&)
  {
    THROW_EXCEPTION(NotExistingSingleton);
  }

  //FIXME need redesign (singleton existence during the
  //method call).
   
  return *_instance;
}

template<class T, int wait_m>
bool SSingleton<T, wait_m>::isConstructed()
{
  return !state_is(*this, S(in_construction));
}

template<class T, int wait_m>
Event SSingleton<T, wait_m>::is_complete()
{
  static std::once_flag once;
  static std::unique_ptr<Event> is_complete_event_ptr;

  //FIXME once
  std::call_once(once, []()
  {
    is_complete_event_ptr.reset(new Event
      (SFORMAT(typeid(SSingleton<T, wait_m>).name()
             << ":is_complete()::is_complete_event"), 
       true, false));
    // prevent infinit loop in RThreadRepository::current
    is_complete_event_ptr->log_params().wait = false;
  });
  return *is_complete_event_ptr;
}

// SAutoSingleton

class SAutoSingletonRegistry
{
public:
  SAutoSingletonRegistry() : ases(500) {}

  //! Deletes all registered singletons in the order
  //! which is opposite to the registration.
  ~SAutoSingletonRegistry() noexcept;

  //! Registers a new singleton, take the ownership (in
  //! means of destruction).
  void reg(SAutoSingletonBase*);

protected:
  boost::lockfree::stack<SAutoSingletonBase*> ases;
};

/*
template<class T, int wait_m>
SAutoSingleton<T, wait_m>::~SAutoSingleton()
{
  extern SAutoSingletonRegistry auto_reg;

  auto_reg.dereg(this);
}
*/

template<class T, int wait_m>
T& SAutoSingleton<T, wait_m>::instance()
{
  construct_once();
  return SSingleton<T, wait_m>::instance ();
}

template<class T, int wait_m>
void SAutoSingleton<T, wait_m>::construct_once()
{
  // It is guaranteed that thee auto_reg will be
  // constructed before this method call.
  extern SAutoSingletonRegistry auto_reg;

  if (!state_is(SSingleton<T, wait_m>::s_the_class(), 
       SSingleton<T, wait_m>::TheClass::exist_oneFun()))
  {
    try 
    { 
      auto_reg.reg(new T()); 
    } 
    catch (const MustBeSingleton&) {}
  }
}

}
#endif

