/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
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
#include "types/exception.h"
#include "types/ext_constr.h"
#include "SSingleton.h"
#include "Logging.h"
#include "RMutex.h"
#include "SCheck.h"
#include "Existent.hpp"

namespace curr {

// SingletonStateHook

template<class T, int wait_m>
void SingletonStateHook<T, wait_m>::operator() 
  (AbstractObjectWithStates* object,
   const StateAxis&,
   const UniversalState& new_state)
{
  using S = SSingleton<T, wait_m>;

  // Unable to cast to Existent because it is not
  // constructed yet
  auto* obj_ptr = dynamic_cast
    <ObjectWithStatesInterface<ExistenceAxis>*>(object);

  SCHECK(obj_ptr);
  auto& obj = *obj_ptr;
  const RState<ExistenceAxis> newst(new_state);

  if (newst == S::TheClass::preinc_exist_severalFun())
      // it is obligatory to check reason of enter to
      // disable intercept preinc_exist_several set by
      // others.
  {
    if(
      RMixedAxis<SingletonAxis, ExistenceAxis>
      ::compare_and_move(
        obj,
        S::TheClass::preinc_exist_severalFun(),
        // <NB> prior obj_count++
        S::TheClass::exist_oneFun()
        // <NB> the singleton constructor will be failed
        ))
      typename S::MustBeSingleton().raise();
    else
      throw ::types::exception<SingletonException>
        ("Program error"); // check the state design if
                           // you get it
  }
  else if (newst == S::TheClass::predec_exist_oneFun())
  {
    // sync the class state with the object state (wait
    // for yields)
    wait_and_move<S>(
      *S::instance_intl(), 
      S::TheClass::instance()->is_exist_one(),
      S::TheClass::predec_exist_oneFun(),
      wait_m
    );
  }
}

// SSingleton

#define SSingletonTW_ SSingleton<T, wait_m>

template<class T, int wait_m>
DEFINE_STATE_CONST(SSingletonTW_, State, preoccupied);
template<class T, int wait_m>
DEFINE_STATE_CONST(SSingletonTW_, State, occupied);
template<class T, int wait_m>
DEFINE_STATE_CONST(SSingletonTW_, State, postoccupied);

template<class T, int wait_m>
void SSingleton<T, wait_m>::NotExistingSingleton
//
::raise() const
{
  using namespace ::types;
  throw ::types::exception(
    *this,
    "Somebody tries to get an ", 
    limit_head<60>(type<SSingleton>::name()),
    "::instance() when no one is available"
  );
}

template<class T, int wait_m>
void SSingleton<T, wait_m>::MustBeSingleton
//
::raise() const
{
  using namespace ::types;
  throw ::types::exception(
    *this,
    "Somebody tries to create more than one ",
    limit_head<60>(type<SSingleton>::name()),
    " instance"
  );
}

//FIXME concurrency problems still here

template<class T, int wait_m>
T* SSingleton<T, wait_m>::_instance; // <NB> no explicit init

template<class T, int wait_m>
SSingleton<T, wait_m>::SSingleton()
  : /*RStateSplitter<SingletonAxis, ExistenceAxis>(
      SSingleton<T, wait_m>::TheClass::instance(),
      SSingleton<T, wait_m>::TheClass::preinc_exist_oneFun()
    ),*/
    CONSTRUCT_EVENT(occupied)
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
    // FIXME add a guard
//    is_complete().reset();
    _instance = nullptr;
  }
}

template<class T, int wait_m>
void SSingleton<T, wait_m>::complete_construction()
{
  if ((_instance = dynamic_cast<T*>(this))) 
    Parent::complete_construction();
}

// No logging here (recursion is possible)
template<class T, int wait_m>
T* SSingleton<T, wait_m>::instance_intl()
{
  // If another thread starts creating T we must wait the
  // completion of the creation
  if (!is_exist_one().wait(wait_m))
    NotExistingSingleton().raise();

  return _instance;
}

template<class T, int wait_m>
bool SSingleton<T, wait_m>::isConstructed()
{
  return !state_is(*this, ConstructibleObject::preinc_exist_oneFun());
}

template<class T, int wait_m>
void SSingleton<T, wait_m>::occupy() const
{
  using log = Logger<LOG::Singletons>;

  using S = SSingleton<T, wait_m>;

  wait_and_move(
    const_cast<S&>(*this),
    { S::TheClass::exist_oneFun(), occupiedState },
    is_occupiable_event,
    preoccupiedState,
    wait_m
  );

  ++occupy_count;
  LOG_TRACE(log, "++occupty_count == " << occupy_count);
  assert(occupy_count > 0);

  move_to(const_cast<S&>(*this), occupiedState);
}

template<class T, int wait_m>
void SSingleton<T, wait_m>::yield() const
{
  using log = Logger<LOG::Singletons>;
  using S = SSingleton<T, wait_m>;

  wait_and_move(
    const_cast<S&>(*this),
    is_occupied_event,
    postoccupiedState,
    wait_m
  );

  assert(occupy_count > 0);
  --occupy_count;
  LOG_TRACE(log, "--occupty_count == " << occupy_count);

  if (occupy_count == 0)
    move_to(
      const_cast<S&>(*this), 
      S::TheClass::exist_oneFun()
    );
  else
    move_to(const_cast<S&>(*this), occupiedState);
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

template<class T, bool destruct, int wait_m>
typename SSingleton<T, wait_m>::Holder 
SAutoSingleton<T, destruct, wait_m>::instance()
{
  construct_once();
  return SSingleton<T, wait_m>::instance ();
}

template<class T, bool destruct, int wait_m>
void SAutoSingleton<T, destruct, wait_m>::construct_once()
{
  // It is guaranteed that thee auto_reg will be
  // constructed before this method call.
  extern ::types::externally_constructed
    <SAutoSingletonRegistry> auto_reg;

  if (!state_is(SSingleton<T, wait_m>::s_the_class(), 
       SSingleton<T, wait_m>::TheClass::exist_oneFun()))
  {
    try 
    { 
      auto_reg.m.reg(new T()); 
    } 
    catch (const MustBeSingleton&) {}
  }
}

}
#endif

