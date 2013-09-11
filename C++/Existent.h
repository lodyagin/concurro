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

#ifndef CONCURRO_EXISTENT_H_
#define CONCURRO_EXISTENT_H_

#include "ClassWithStates.h"
#include "StateAxis.h"
#include <functional>

namespace curr {

//! @addtogroup Singletons
//! @{

DECLARE_AXIS(ExistenceAxis, StateAxis);

extern char existent_class_initial_state[];

/**
 * Every object has two main states - exist and not_exist
 * which forms the ExistenceAxis. An object should be
 * derived from Existent if we need observe class 
 * existence (e.g., in SSingleton).
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   not_exist [shape = doublecircle];
 *   start -> not_exist;
 *   not_exist -> preinc_exist_one
 *     [label="inc_existence()"];
 *   preinc_exist_one -> exist_one
 *     [label="inc_existence()"];
 *   exist_one -> preinc_exist_several
 *     [label="inc_existence()"];
 *   preinc_exist_several -> exist_several
 *     [label="inc_existence()"];
 *   exist_several -> preinc_exist_several
 *     [label="inc_existence()"];
 *   exist_several -> predec_exist_several
 *     [label="dec_existence()"];
 *   predec_exist_several -> exist_several
 *     [label="dec_existence()"];
 *   predec_exist_several -> exist_one
 *     [label="dec_existence()"];
 *   exist_one -> predec_exist_one
 *     [label="dec_existence()"];
 *   predec_exist_one -> not_exist
 *     [label="dec_existence()"];
 * }
 * @enddot
 *
 * State constants are declared separately in
 * ExistentState class to prevent cyclic dependencies.
 */
template<
  class T, 
  class StateHook = EmptyStateHook
>
class Existent 
: public ClassWithStates
  < T,
    ExistenceAxis, 
    existent_class_initial_state,
    StateHook
  >
{
public:

  Existent();
  Existent(const Existent&);
  Existent(Existent&&);
  virtual ~Existent();
  Existent& operator=(const Existent&);
  Existent& operator=(Existent&&);

  //! Return an empty event (no object-observable terminal
  //! state).
  curr::CompoundEvent is_terminal_state() const override
  {
    return curr::CompoundEvent();
  }

  //! Return a number of Existent objects
  static unsigned get_obj_count()
  { return obj_count; }

protected:
  static std::atomic<int> obj_count;

  void inc_existence();
  void dec_existence();
};

//! @}

}

#endif
