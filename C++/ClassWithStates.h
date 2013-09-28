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

#ifndef CONCURRO_CLASSWITHSTATES_H_
#define CONCURRO_CLASSWITHSTATES_H_

#include "ObjectWithStatesInterface.h"

namespace curr {

//! @addtogroup states
//! @{

template<class T, class Axis, const char* initial,
        class StateHook
>
class ClassWithStates
{
public:
  virtual ~ClassWithStates() {}

  ClassWithStates* get_paired() //const
  { return paired; }

protected:
  /** States implementation. It is in an inner class
   * because we won't mix class states with object states
   * (class states are like state of repository or another
   * structure compound from separate members - classes.
   */
  class TheClass: public ObjectWithStatesInterface<Axis>
  {
  public:
    TheClass(ClassWithStates* inst)
      : instance(inst) { assert(instance); }
    
    void state_changed
      (StateAxis& ax, 
       const StateAxis& state_ax,     
       AbstractObjectWithStates* object,
       const UniversalState& new_state) override;

    std::atomic<uint32_t>& 
      current_state(const StateAxis& ax) override;

    const std::atomic<uint32_t>& 
      current_state(const StateAxis& ax) const override;

#if 0
    //! Return an empty event (no object-observable terminal
    //! state).
    curr::CompoundEvent is_terminal_state() const override
    {
      return curr::CompoundEvent();
    }
#endif

  protected:
    //! Instance of ClassWithStates
    ClassWithStates* instance;
  };

  //! Paired obj ptr, for example, in the middle of a move
  //! constructor. It's used in descendants but declared
  //! here to access from StateHook-s (without dynamic
  //! casting).
  ClassWithStates* paired = nullptr;
};

template<class T, class Axis, const char* initial>
class EmptyStateHook
{
public:
  EmptyStateHook
    (ClassWithStates<T, Axis, initial, EmptyStateHook>*) 
  {}

  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state)
  {}
};

//! @}

}
#endif


