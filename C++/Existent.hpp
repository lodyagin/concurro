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

#ifndef CONCURRO_EXISTENT_HPP_
#define CONCURRO_EXISTENT_HPP_

#include "Existent.h"
#include "RState.h"
#include "ClassWithStates.hpp"

namespace curr {

class ExistentStates
{
public:
  //! @cond
  DECLARE_STATES(ExistenceAxis, State);
  DECLARE_STATE_FUN(State, not_exist);
  DECLARE_STATE_FUN(State, predec_exist_one);
  DECLARE_STATE_FUN(State, preinc_exist_one);
  DECLARE_STATE_FUN(State, exist_one);
  DECLARE_STATE_FUN(State, exist_several);
  DECLARE_STATE_FUN(State, predec_exist_several);
  DECLARE_STATE_FUN(State, preinc_exist_several);
  DECLARE_STATE_FUN(State, moving_when_one);
  DECLARE_STATE_FUN(State, moving_when_several);
  //! @endcond
};

template<class T, class StateHook>
std::atomic<int> Existent<T, StateHook>::obj_count(0);

template<class T, class StateHook>
Existent<T, StateHook>::Existent() : theClass(this)
{
  inc_existence();
}

template<class T, class StateHook>
Existent<T, StateHook>::Existent(const Existent&) 
  : theClass(this)
{
  inc_existence();
}

template<class T, class StateHook>
Existent<T, StateHook>::Existent(Existent&& e) 
  : theClass(this)
{
  while(!(ExistentStates::State::compare_and_move(
            *this,
            ExistentStates::exist_oneFun(),
            ExistentStates::moving_when_oneFun())
          || 
          ExistentStates::State::compare_and_move(
            *this,
            ExistentStates::exist_severalFun(),
            ExistentStates::moving_when_severalFun())));
  e.paired = this;
}

template<class T, class StateHook>
Existent<T, StateHook>::~Existent()
{
  if (paired &&
      // the next is performed only in a destructor of
      // moved object
      (ExistentStates::State::compare_and_move(
        this->theClass,
        ExistentStates::moving_when_oneFun(),
        ExistentStates::exist_oneFun())
       ||
       ExistentStates::State::compare_and_move(
         this->theClass,
         ExistentStates::moving_when_severalFun(),
         ExistentStates::exist_severalFun()))
    )
    ; // <NB> the object was moved, the quantity wasn't
      // changed
  else
    dec_existence();
}

template<class T, class StateHook>
Existent<T, StateHook>& Existent<T, StateHook>
::operator=(const Existent& e)
{
  theClass = e.theClass;
  inc_existence();
}

template<class T, class StateHook>
Existent<T, StateHook>& Existent<T, StateHook>
::operator=(Existent&& e)
{
  theClass = e.theClass;

  while(!(ExistentStates::State::compare_and_move(
            this->theClass,
            ExistentStates::exist_oneFun(),
            ExistentStates::moving_when_oneFun())
          || 
          ExistentStates::State::compare_and_move(
            this->theClass,
            ExistentStates::exist_severalFun(),
            ExistentStates::moving_when_severalFun())))
    ;

  e.paired = this;
}

template<class T, class StateHook>
void Existent<T, StateHook>::inc_existence()
{
  bool b = false, a;

  while (
    ! ((a = ExistentStates::State::compare_and_move(
       this->theClass, 
       ExistentStates::not_existFun(), 
       ExistentStates::preinc_exist_oneFun()))
     || 
       (b = ExistentStates::State::compare_and_move(
       this->theClass, 
       { ExistentStates::exist_oneFun(), 
         ExistentStates::exist_severalFun() 
       }, 
       ExistentStates::preinc_exist_severalFun()))
      )
   );

  obj_count++;
  assert (obj_count > 0);
  assert ((obj_count == 1) == a); 
  assert ((obj_count > 1) == b);

  if (a) 
    ExistentStates::State::move_to(
      this->theClass, ExistentStates::exist_oneFun());
  else if (b) 
    ExistentStates::State::move_to(
      this->theClass, ExistentStates::exist_severalFun());
}

template<class T, class StateHook>
void Existent<T, StateHook>::dec_existence()
{
  bool b = false, a;

  while (
   ! ((a = ExistentStates::State::compare_and_move(
     this->theClass, 
     ExistentStates::exist_severalFun(), 
     ExistentStates::predec_exist_severalFun()))
   || (b = ExistentStates::State::compare_and_move(
       this->theClass, 
       ExistentStates::exist_oneFun(), 
       ExistentStates::predec_exist_oneFun()))
     )
   );

  obj_count--;
  assert (obj_count >= 0);

  if (obj_count == 1)
    ExistentStates::State::move_to(
      this->theClass, ExistentStates::exist_oneFun());
  else if (a)
    ExistentStates::State::move_to(
      this->theClass, ExistentStates::exist_severalFun());
  else if (b)
    ExistentStates::State::move_to(
      this->theClass, ExistentStates::not_existFun());
}

}
#endif
