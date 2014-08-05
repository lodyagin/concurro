/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
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

#include <functional>
#include "ClassWithStates.h"
#include "RState.h"
#include "REvent.h"

namespace curr {

//! @addtogroup singletons
//! @{

DECLARE_AXIS(ExistenceAxis, StateAxis);

extern ::types::constexpr_string existent_class_initial_state;

template<class T>
using ExistentEmptyStateHook = EmptyStateHook
  <ExistenceAxis, existent_class_initial_state>;

/**
 * Every class has two main states - exist and not_exist
 * which forms the ExistenceAxis. A class should be
 * derived from Existent if we need observe
 * existence of its copies (e.g., in SSingleton).
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   not_exist [shape = doublecircle];
 *   start -> not_exist;
 *   not_exist -> preinc_exist_one
 *     [label="inc_existence()"];
 *   preinc_exist_one -> exist_one
 *     [label="complete_construction()"];
 *   exist_one -> preinc_exist_several
 *     [label="inc_existence()"];
 *   preinc_exist_several -> exist_several
 *     [label="complete_construction()"];
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
 *   exist_one -> moving_when_one
 *     [label="std::move"];
 *   moving_when_one -> exist_one
 *     [label="std::move"];
 *   exist_several -> moving_when_several
 *     [label="std::move"];
 *   moving_when_several -> exist_several
 *     [label="std::move"];
 * }
 * @enddot
 *
 * State constants are declared separately in
 * ExistentState class to prevent cyclic dependencies.
 */
template<
  class T, 
  class StateHook = ExistentEmptyStateHook<T>
>
class Existent 
  : public ClassWithEvents
    < ExistenceAxis, 
      existent_class_initial_state,
      StateHook
    >,
    public virtual CompleteConstruction
{
public:
  using Parent = ClassWithEvents
    <ExistenceAxis, existent_class_initial_state,
     StateHook>;

  class TheClass : public Parent::TheClass
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

//    event_fun<ExistenceAxis> is_not_exist;

    CompoundEvent is_terminal_state() const override
    {
//      return is_not_exist();
      return CompoundEvent();
    }

    log4cxx::LoggerPtr logger() const override
    {
      return nullptr; // disable logging to prevent
                      // a deadlock
    }

    static TheClass* instance() 
    { 
      static std::once_flag of;
      static TheClass* i = nullptr;

      // pass i as a parementer due to a GCC bug
      std::call_once(of, [](TheClass** i)
      { 
        *i = new TheClass(); 
      }, &i);
      assert(i);
      return i; 
    }

  private:
    TheClass() {}
    TheClass(const TheClass&) = delete;
    ~TheClass() {}
    TheClass& operator=(const TheClass&) = delete;
  };


  Existent();
  Existent(const Existent&) = delete;

  //! It leaves the object in moving_when_one or
  //! moving_when several state. Must be completed in
  //! parent (-> {exist_one, exist_everal}).
  Existent(Existent&&) = delete;

  virtual ~Existent();
  Existent& operator=(const Existent&) = delete;
  Existent& operator=(Existent&&) = delete;

  //! Return a number of Existent objects
  static unsigned get_obj_count()
  { return obj_count; }

  static TheClass& s_the_class()
  {
    return *TheClass::instance();
  }

  typename ClassWithStates <
    ExistenceAxis, 
    existent_class_initial_state,
    StateHook>
  ::TheClass& the_class() const override
  {
    return s_the_class();
  }

  void complete_construction() override;

protected:
  static std::atomic<int> obj_count;

  void inc_existence();
  void dec_existence();

private:
//  typedef Logger<Existent> log;
};

//! @}

}

#endif
