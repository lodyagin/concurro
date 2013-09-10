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
#include "RObjectWithStates.hpp"
#include "RState.hpp"

namespace curr {

template<class T>
DEFINE_STATE_CONST(Existent<T>, State, not_exist);

template<class T>
DEFINE_STATE_CONST(Existent<T>, State, pre_exist_one);

template<class T>
DEFINE_STATE_CONST(Existent<T>, State, exist_one);

template<class T>
DEFINE_STATE_CONST(Existent<T>, State, exist_several);

template<class T>
DEFINE_STATE_CONST(Existent<T>, State, pre_exist_several);

template<class T>
std::atomic<int> Existent<T>::obj_count(0);

template<class T>
Existent<T>::Existent()
{
  inc_existence();
}

template<class T>
Existent<T>::Existent(const Existent&)
{
  inc_existence();
}

template<class T>
Existent<T>::Existent(Existent&&)
{
}

template<class T>
Existent<T>::~Existent()
{
  dec_existence();
}

template<class T>
Existent<T>& Existent<T>::operator=(const Existent&)
{
  inc_existence();
}

template<class T>
Existent<T>& Existent<T>::operator=(Existent&&)
{
}

template<class T>
void Existent<T>::inc_existence()
{
  bool b = false, a;

  while (
   ! ((a = State::compare_and_move(
       *this, not_existState, pre_exist_oneState))
     || (b = State::compare_and_move(
       *this, exist_oneState, pre_exist_severalState))
     || State::compare_and_move(
       *this, exist_severalState, exist_severalState)
     )
   );

  obj_count++;
  assert (obj_count > 0);
  assert ((obj_count == 1) == a); 
  assert (IMPLICATION(b, obj_count > 1));

  if (a) State::move_to(*this, exist_oneState);
  else if (b) State::move_to(*this, exist_severalState);
}

template<class T>
void Existent<T>::dec_existence()
{
  bool b = false, a;

  while (
   ! ((a = State::compare_and_move(
     *this, exist_severalState, pre_exist_severalState))
     || (b = State::compare_and_move(
       *this, exist_oneState, pre_exist_oneState))
     )
   );

  obj_count--;
  assert (obj_count >= 0);

  if (obj_count == 1)
    State::move_to(*this, exist_oneState);
  else if (a)
    State::move_to(*this, exist_severalState);
  else if (b)
    State::move_to(*this, not_existState);
}

}
#endif
