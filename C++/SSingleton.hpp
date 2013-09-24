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

#include "SSingleton.h"
#include "Logging.h"
#include "SCheck.h"
#include "Existent.hpp"
#include <mutex>

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

template<class T>
typename SingletonStateHook<T>::Instance* 
SingletonStateHook<T>::instance; // <NB> no explicit init

template<class T>
SingletonStateHook<T>
::SingletonStateHook(Instance* inst)
: last_instance(inst)
{
  SCHECK(last_instance);
}

template<class T>
void SingletonStateHook<T>::operator() 
  (AbstractObjectWithStates* object,
   const StateAxis&,
   const UniversalState& new_state)
{
  auto* obj_ptr = dynamic_cast
    <ObjectWithStatesInterface<ExistenceAxis>*>(object);
  SCHECK(obj_ptr);
  auto& obj = *obj_ptr;
  const RState<ExistenceAxis> newst(new_state);

  if (newst == ExistentStates::preinc_exist_severalFun())
      // it is obligatory to check reason of enter to
      // disable intercept preinc_exist_several set by
      // others.
  {
    if(
      RMixedAxis<SingletonAxis, ExistenceAxis>
      ::compare_and_move(
        obj,
        ExistentStates::preinc_exist_severalFun(),
        // <NB> prior obj_count++
        ExistentStates::exist_oneFun()
        // <NB> constructor will be failed
        ))
      THROW_EXCEPTION(MustBeSingleton);
    else
      THROW_PROGRAM_ERROR; // check the state design if
                           // you get it
  }
  else if (newst == ExistentStates::exist_oneFun())
  {
    auto* p = dynamic_cast<SSingleton<T>*>(last_instance);
    if (p->paired)
      instance = p->paired;
    // move the instance in move construction/assignment
  }

  if (RMixedAxis<SingletonAxis, ExistenceAxis>
      ::state_is(
        obj, ExistentStates::preinc_exist_oneFun())
    ) 
  {
    SCHECK(!instance);
    instance = last_instance;
  }
  else if (RMixedAxis<SingletonAxis, ExistenceAxis>
           ::state_is(
             obj, ExistentStates::predec_exist_oneFun())
    ) 
  {
    SCHECK(instance);
    instance = nullptr;
  }
}

// SSingleton

//FIXME concurrency problems still here

template<class T>
typename SingletonStateHook<T>::Instance* 
SSingleton<T>::instance0; // <NB> no explicit init

template<class T>
T* SSingleton<T>::_instance; // <NB> no explicit init

template<class T>
SSingleton<T>::SSingleton()
{
  assert(this->get_obj_count() == 1);
  set_instance(SingletonStateHook<T>::instance);
  assert(instance0);
}

template<class T>
SSingleton<T>::~SSingleton()
{
  assert(this->get_obj_count() == 1);
  assert(instance0);
  instance0 = _instance = nullptr;
}

template<class T>
inline T & SSingleton<T>::instance()
{
  if (!instance0)
    THROW_EXCEPTION(NotExistingSingleton);
  else if (!_instance)
    _instance = dynamic_cast<T*>(instance0);
  SCHECK(_instance);
 
  //FIXME need redesign (singleton existence during the
  //method call).
   
  return *_instance;
}

template<class T>
bool SSingleton<T>::isConstructed()
{
  return instance0;
}

// SAutoSingleton

template<class T>
T& SAutoSingleton<T>::instance()
{
  if (!SSingleton<T>::isConstructed())
  {
    try 
    { 
      new T(); 
    } 
    catch (const MustBeSingleton&) {}
  }

  // FIXME need to wait a complete construction
  return SSingleton<T>::instance ();
}

}
#endif

