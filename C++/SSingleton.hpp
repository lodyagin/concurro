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
void SingletonStateHook<T>::operator() 
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

  if (newst == SSingleton<T>::TheClass
        ::preinc_exist_severalFun())
      // it is obligatory to check reason of enter to
      // disable intercept preinc_exist_several set by
      // others.
  {
    if(
      RMixedAxis<SingletonAxis, ExistenceAxis>
      ::compare_and_move(
        obj,
        SSingleton<T>::TheClass::preinc_exist_severalFun(),
        // <NB> prior obj_count++
        SSingleton<T>::TheClass::exist_oneFun()
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

template<class T>
T* SSingleton<T>::_instance; // <NB> no explicit init

template<class T>
SSingleton<T>::SSingleton()
{
  assert(this->get_obj_count() == 1);
}

#if 0
template<class T>
SSingleton<T>::SSingleton(SSingleton&& s)
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

template<class T>
SSingleton<T>::~SSingleton()
{
  if (!this->paired)
  {
    // do not call it for moved-from objects (that have
    // paired pointed to a new moved-to instance)

    SCHECK(this->get_obj_count() == 1);
    _instance = nullptr;
  }
}

template<class T>
inline T & SSingleton<T>::instance()
{
  if (!_instance)
    THROW_EXCEPTION(NotExistingSingleton);
 
  //FIXME need redesign (singleton existence during the
  //method call).
   
  return *_instance;
}

template<class T>
bool SSingleton<T>::isConstructed()
{
  return !state_is(*this, S(in_construction));
}

// SAutoSingleton

template<class T, int wait_m>
T& SAutoSingleton<T, wait_m>::instance()
{
  if (!state_is(SSingleton<T>::the_class(), 
                SSingleton<T>::TheClass::exist_oneFun()))
  {
    try 
    { 
      new T(); 
    } 
    catch (const MustBeSingleton&) {}
  }

  // If another thread starts creating T we must wait the
  // completion of the creation
  CURR_WAIT(SSingleton<T>::instance()
            . E(complete_construction), wait_m);

  return SSingleton<T>::instance ();
}

}
#endif

